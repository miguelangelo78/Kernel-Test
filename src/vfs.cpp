#include <fs.h>
#include <libc/tree.h>
#include <libc/list.h>
#include <libc/hashmap.h>
#include <errno.h>
#include <system.h>
#include <module.h>

/***********************************************************/
/************* VFS Macros / definitions ********************/
/***********************************************************/
#define MAX_SYMLINK_DEPTH 8
#define MAX_SYMLINK_SIZE 4096

tree_t * fs_tree     = 0; /* Filesystem mountpoint tree */
hashmap_t * fs_types = 0;
FILE * fs_root       = 0; /* Pointer to the root mount fs_node (must be some form of filesystem, even ramdisk) */
static spin_lock_t tmp_refcount_lock = { 0 };
static spin_lock_t tmp_vfs_lock = { 0 };

struct parent_path_packet {
	FILE * parent;
	char * f_path;
};


/***********************************************/
/************* Function Prototypes *************/
/***********************************************/
char * canonicalize_path(char * cwd, char * input);
struct parent_path_packet get_parent(char * path);
static struct dirent * readdir_mapper(FILE *node, uint32_t index);
static FILE * vfs_mapper(void);
FILE * get_mount_point(char * path, unsigned int path_depth, char **outpath, unsigned int * outdepth);
FILE * kopen_recur(char *filename, uint32_t flags, uint32_t symlink_depth, char *relative_to);


/****************************************************************/
/************* VFS Installers/Initializers/Mounters *************/
/****************************************************************/
void vfs_install(void) {
	/* Initialize filesystem tree: */
	fs_tree = tree_create();

	/* Create VFS root: */
	struct vfs_entry * root = (struct vfs_entry *)malloc(sizeof(struct vfs_entry));
	root->name = strdup("[root]");
	root->file = 0; /* Nothing mounted as root (yet) */
	tree_set_root(fs_tree, root);
	fs_types = hashmap_create(5);
}

void vfs_install(uintptr_t initrd_location) {
	vfs_install();
	map_vfs_directory("/dev"); /* Create/map '/dev' directory */
	vfs_mount("/dev/ram1", initrd_init(*(uint32_t*)initrd_location)); /* Mount initrd into '/dev/ram1' */
}

void * vfs_mount(char * path, FILE * local_root) {
	kprintf("\n\t> VFS: Mounting '%s'\n", path);
	if(!fs_tree) {
		kprintfc(COLOR_BAD, "\n\tERROR: VFS hasn't been initialized, you can't mount things yet!");
		return NULL;
	}

	if(!path || path[0] != '/') {
		kprintf(COLOR_BAD, "\n\tERROR: Path must be absolute for mountpoint.");
		return NULL;
	}

	/**** Mount local_root into path: ****/
	spin_lock(tmp_vfs_lock);
	local_root->refcount = -1;

	tree_node_t * ret_val = 0;

	char * p = strdup(path);
	char * i = p;

	int path_len = strlen(p);

	/* Chop the path up. E.g.: /home/usr => \0home\0usr */
	while(i < p + path_len) {
		if(*i == PATH_SEPARATOR)
			*i = '\0';
		i++;
	}
	/* Clean up the end of the string: */
	p[path_len] = '\0';
	i = p + 1; /* skip the first \0 */

	/* Root node: */
	tree_node_t * root_node = fs_tree->root;

	if(*i == '\0') {
		/* Special case, we're trying to set the root node */
		struct vfs_entry * root = (struct vfs_entry *)root_node->value;
		root->file = local_root;
		fs_root = local_root;
		ret_val = root_node;
	} else {
		/* Mount local root into directory: */
		tree_node_t * node = root_node;
		char * at = i;
		while(1) {
			if(at >= p + path_len)
				break;

			char found = 0;
			foreach(child, node->children) {
				tree_node_t * tchild = (tree_node_t *)child->value;
				struct vfs_entry * ent = (struct vfs_entry *)tchild->value;
				if(!strcmp(ent->name, at)) {
					found = 1;
					node = tchild;
					ret_val = node;
					break;
				}
			}

			if(!found) {
				/* Making directory: */
				struct vfs_entry * ent = (struct vfs_entry*)malloc(sizeof(struct vfs_entry));
				ent->name = strdup(at);
				ent->file = 0;
				node = tree_node_insert_child(fs_tree, node, ent);
			}
			at += strlen(at) + 1;
		}

		/* Set local root into the node: */
		struct vfs_entry * ent = (struct vfs_entry *)node->value;
		ent->file = local_root;
		ret_val = node;
	}

	free(p);
	spin_unlock(tmp_vfs_lock);
	return 0;
}
EXPORT_SYMBOL(vfs_mount);

/* Register new filesystem in the VFS: */
int vfs_register(char * filesystem_name, vfs_mount_callback * cback) {
	if(hashmap_get(fs_types, filesystem_name)) return 1;
	hashmap_set(fs_types, filesystem_name, (void*)(uintptr_t)(cback));
	return 0;
}
EXPORT_SYMBOL(vfs_register);

/* Mount filesystem (EXT2, FAT32, ...) into path (mountpoint) */
int vfs_mount_type(char * type, char * arg, char * mountpoint) {
	vfs_mount_callback cback = (vfs_mount_callback)(uintptr_t)hashmap_get(fs_types, type);
	if(!cback) {
		kprintf(COLOR_BAD, "Unknown filesystem: %s", type);
		return -ENODEV;
	}

	/* Mount new filesystem into the VFS: */
	FILE * n = cback(arg, mountpoint);
	if(!n) return -EINVAL;
	vfs_mount(mountpoint, n);

	kprintfc(COLOR_INFO, "\n\tMounted %s[%s] to %s: 0x%x", type, arg, mountpoint, n);
	return 0;
}
EXPORT_SYMBOL(vfs_mount_type);

void vfs_lock(FILE * node) {
	spin_lock(tmp_refcount_lock);
	node->refcount = -1;
	spin_unlock(tmp_refcount_lock);
}
EXPORT_SYMBOL(vfs_lock);

/* Create virtual node/directory on the VFS: */
void map_vfs_directory(char * c) {
	FILE * f = vfs_mapper();
	struct vfs_entry * e = (struct vfs_entry *)vfs_mount(c, f);
	f->device = !strcmp(c, "/") ? (void*)fs_tree->root : (void*)e;
}
EXPORT_SYMBOL(map_vfs_directory);


/*****************************************/
/************* VFS Functions *************/
/*****************************************/
char fs_is_dir(FILE * node) {
	/* Is node a directory: */
	return (node->flags & 0x7) == FS_DIR;
}
EXPORT_SYMBOL(fs_is_dir);

uint32_t fread(FILE * node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	return node && node->read  ? node->read(node, offset, size, buffer)  : -1;
}
EXPORT_SYMBOL(fread);

uint32_t fwrite(FILE * node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	return node && node->write ? node->write(node, offset, size, buffer) : -1;
}
EXPORT_SYMBOL(fwrite);

uint32_t fopen(FILE * node, unsigned int flags) {
	if(!node) return -1;

	if(node->refcount >= 0) {
		spin_lock(tmp_refcount_lock);
		node->refcount++;
		spin_unlock(tmp_refcount_lock);
	}
	return node->open ? node->open(node, flags) : -1;
}
EXPORT_SYMBOL(fopen);

uint32_t fclose(FILE * node) {
	if(node) {
		if(node->refcount == -1) {
			return -1;
		}
		else {
			spin_lock(tmp_refcount_lock);
			if(--node->refcount == 0) {
				if(node->close)
					node->close(node);
				free(node);
			}
			spin_unlock(tmp_refcount_lock);
			return 0;
		}
	} else {
		return -1;
	}
}
EXPORT_SYMBOL(fclose);

struct dirent * fs_readdir(FILE* node, uint32_t index) {
	return (node && fs_is_dir(node) && node->readdir) ? node->readdir(node, index) : 0;
}
EXPORT_SYMBOL(fs_readdir);

FILE * fs_finddir(FILE* node, char * name) {
	return (node && fs_is_dir(node) && node->finddir) ? node->finddir(node, name)  : 0;
}
EXPORT_SYMBOL(fs_finddir);

uint32_t fs_filesize(FILE * node) {
	return node->size;
}
EXPORT_SYMBOL(fs_filesize);

int fs_mkdir(char * dirname, uint16_t permission) {
	char * canon_path = canonicalize_path(current_task->work_dirpath, dirname);
	struct parent_path_packet packet = get_parent(canon_path);

	if(packet.parent && packet.parent->mkdir) {
		packet.parent->mkdir(packet.parent, packet.f_path, permission);
		free(canon_path);
		fclose(packet.parent);
		return 0;
	} else {
		free(canon_path);
		if(packet.parent) free(packet.parent);
		return -1;
	}
}
EXPORT_SYMBOL(fs_mkdir);

int fs_create_file(char * filename, uint16_t permission) {
	char * canon_path = canonicalize_path(current_task->work_dirpath, filename);
	struct parent_path_packet packet = get_parent(canon_path);

	if(packet.parent && packet.parent->create) {
		packet.parent->create(packet.parent, packet.f_path, permission);
		free(canon_path);
		free(packet.parent);
		return 0;
	} else {
		free(canon_path);
		if(packet.parent) free(packet.parent);
		return -1;
	}
}
EXPORT_SYMBOL(fs_create_file);

FILE * kopen(char * filename, uint32_t flags) {
	return kopen_recur(filename, flags, 0, current_task->work_dirpath);
}
EXPORT_SYMBOL(kopen);

FILE * fs_clone(FILE * source) {
	if(!source) return 0;
	if(source->refcount >= 0) {
		spin_lock(tmp_refcount_lock);
		source->refcount++;
		spin_unlock(tmp_refcount_lock);
	}
	return source;
}
EXPORT_SYMBOL(fs_clone);

int fs_ioctl(FILE * node, int request, void * argp) {
	return node && node->ioctl ? node->ioctl(node, request, argp) : -1;
}
EXPORT_SYMBOL(fs_ioctl);

int fs_chmod(FILE * node, int mode) {
	return node && node->chmod ? node->chmod(node, mode) : -1;
}
EXPORT_SYMBOL(fs_chmod);

int fs_unlink(char * filename) {
	char * canon_path = canonicalize_path(current_task->work_dirpath, filename);
	struct parent_path_packet packet = get_parent(canon_path);

	if(packet.parent && packet.parent->unlink) {
		packet.parent->unlink(packet.parent, packet.f_path);
		free(canon_path);
		free(packet.parent);
		return 0;
	} else {
		free(canon_path);
		if(packet.parent) free(packet.parent);
		return -1;
	}
}
EXPORT_SYMBOL(fs_unlink);

int fs_symlink(char * target, char * filename) {
	char * canon_path = canonicalize_path(current_task->work_dirpath, filename);
	struct parent_path_packet packet = get_parent(canon_path);

	if(packet.parent && packet.parent->symlink) {
		packet.parent->symlink(packet.parent, target, packet.f_path);
		free(canon_path);
		fclose(packet.parent);
		return 0;
	} else {
		free(canon_path);
		if(packet.parent) free(packet.parent);
		return -1;
	}
}
EXPORT_SYMBOL(fs_symlink);

int fs_readlink(FILE * node, char * buff, size_t size) {
	return node && node->readlink ? node->readlink(node, buff, size) : -1;
}
EXPORT_SYMBOL(fs_readlink);


/********************************************************/
/************* VFS Implementation Functions *************/
/********************************************************/
char * canonicalize_path(char * cwd, char * input) {
	/* This is a stack-based canonicalizer; we use a list as a stack */
	list_t * out = list_create();

	/*
	* If we have a relative path, we need to canonicalize
	* the working directory and insert it into the stack.
	*/
	if(strlen(input) && input[0] != PATH_SEPARATOR) {
		/* Make a copy of the working directory */
		char * path = (char*)malloc((strlen(cwd) + 1) * sizeof(char));
		memcpy(path, cwd, strlen(cwd) + 1);

		/* Setup tokenizer */
		char * pch;
		char * save;
		pch = strtok_r(path, PATH_SEPARATOR_STRING, &save);

		/* Start tokenizing */
		while(pch != 0) {
			/* Make copies of the path elements */
			char * s = (char*)malloc(sizeof(char) * (strlen(pch) + 1));
			memcpy(s, pch, strlen(pch) + 1);
			/* And push them */
			list_insert(out, s);
			pch = strtok_r(0, PATH_SEPARATOR_STRING, &save);
		}
		free(path);
	}

	/* Similarly, we need to push the elements from the new path */
	char * path = (char*)malloc((strlen(input) + 1) * sizeof(char));
	memcpy(path, input, strlen(input) + 1);

	/* Initialize the tokenizer... */
	char *pch;
	char *save;
	pch = strtok_r(path,PATH_SEPARATOR_STRING,&save);

	/*
	 * Tokenize the path, this time, taking care to properly
	 * handle .. and . to represent up (stack pop) and current
	 * (do nothing)
	 */
	while(pch != 0) {
		if (!strcmp(pch,PATH_UP)) {
			/*
			* Path = ..
			* Pop the stack to move up a directory
			*/
			node_t * n = list_pop(out);
			if (n) {
				free(n->value);
				free(n);
			}
		} else if (!strcmp(pch,PATH_DOT)) {
			/*
			* Path = .
			* Do nothing
			*/
		} else {
			/*
			* Regular path, push it
			* XXX: Path elements should be checked for existence!
			*/
			char * s = (char*)malloc(sizeof(char) * (strlen(pch) + 1));
			memcpy(s, pch, strlen(pch) + 1);
			list_insert(out, s);
		}
		pch = strtok_r(0, PATH_SEPARATOR_STRING, &save);
	}
	free(path);

	/* Calculate the size of the path string */
	size_t size = 0;
	foreach(item, out) {
		/* Helpful use of our foreach macro. */
		size += strlen((char*)item->value) + 1;
	}

	/* join() the list */
	char *output = (char*)malloc(sizeof(char) * (size + 1));
	char *output_offset = output;
	if(size == 0) {
		/*
		 * If the path is empty, we take this to mean the root
		 * thus we synthesize a path of "/" to return.
		 */
		output = (char*)realloc(output, sizeof(char) * 2);
		output[0] = PATH_SEPARATOR;
		output[1] = '\0';
	} else {
		/* Otherwise, append each element together */
		foreach(item, out) {
			output_offset[0] = PATH_SEPARATOR;
			output_offset++;
			memcpy(output_offset, item->value, strlen((char*)item->value) + 1);
			output_offset += strlen((char*)item->value);
		}
	}

	/* Clean up the various things we used to get here */
	list_destroy(out);
	list_free(out);
	free(out);

	/* And return a working, absolute path */
	return output;
}
EXPORT_SYMBOL(canonicalize_path);

struct parent_path_packet get_parent(char * path) {
	FILE * parent;
	char * parent_path = (char*)malloc(strlen(path) + 4);
	sprintf(parent_path, "%s/..", path);

	char * f_path = path + strlen(path) - 1;
	while(f_path > path) {
		if(*f_path == '/') {
			f_path++;
			break;
		}
		f_path--;
	}

	parent = kopen(parent_path, 0);
	free(parent_path);

	struct parent_path_packet packet;
	packet.f_path = f_path;
	packet.parent = parent;
	return packet;
}

static struct dirent * readdir_mapper(FILE * node, uint32_t index) {
	tree_node_t * d = (tree_node_t*)node->device;
	if(!d) return 0;

	if(index == 0) {
		/* Path itself */
		struct dirent * dir = (struct dirent *)malloc(sizeof(struct dirent));
		strcpy(dir->name, ".");
		dir->ino = 0;
		return dir;
	} else if(index == 1) {
		/* Parent path */
		struct dirent * dir = (struct dirent *)malloc(sizeof(struct dirent));
		strcpy(dir->name, "..");
		dir->ino = 1;
		return dir;
	}

	index -= 2;

	unsigned int i = 0;
	foreach(child, d->children) {
		if(i == index) {
			tree_node_t * tchild = (tree_node_t *)child->value;
			struct vfs_entry * n = (struct vfs_entry *)tchild->value;

			/* Return dirent: */
			struct dirent * dir = (struct dirent *)malloc(sizeof(struct dirent));
			size_t len = strlen(n->name) + 1;
			memcpy(&dir->name, n->name, MIN(256, len));
			dir->ino = i;
			return dir;
		}
		i++;
	}
	return 0;
}

static FILE * vfs_mapper(void) {
	FILE * node = (FILE*)malloc(sizeof(FILE));
	memset(node, 0, sizeof(FILE));
	node->mask = 0666;
	node->flags = FS_DIR;
	node->readdir = readdir_mapper;
	return node;
}

FILE * get_mount_point(char * path, unsigned int path_depth, char **outpath, unsigned int * outdepth) {
	for(size_t depth = 0; depth <= path_depth; depth++)
		path += strlen(path) + 1;

	FILE * last = fs_root;
	tree_node_t * node = fs_tree->root;

	char * at = *outpath;
	int _depth = 1;
	int _tree_depth = 0;

	while(1) {
		if(at >= path) break;

		int found = 0;
		foreach(child, node->children) {
			tree_node_t * tchild = (tree_node_t*)child->value;
			struct vfs_entry * ent = (struct vfs_entry*)tchild->value;
			if(!strcmp(ent->name, at)) {
				found = 1;
				node = tchild;
				at = at + strlen(at) + 1;
				if(ent->file) {
					_tree_depth = _depth;
					last = ent->file;
					*outpath = at;
				}
				break;
			}
		}
		if(!found) break;
		_depth++;
	}

	*outdepth = _tree_depth;
	if(last) {
		FILE * last_clone = (FILE *)malloc(sizeof(FILE));
		memcpy(last_clone, last, sizeof(FILE));
		return last_clone;
	}
	return last;
}

FILE * kopen_recur(char * filename, uint32_t flags, uint32_t symlink_depth, char * relative_to) {
	if(!filename) return 0;

	char * path = canonicalize_path(relative_to, filename);
	size_t path_len = strlen(path);

	if(path_len == 1) {
		/* Return the node at '/' */
		FILE * root_clone = (FILE*)malloc(sizeof(FILE));
		memcpy(root_clone, fs_root, sizeof(FILE));
		free(path);
		fopen(root_clone, flags);
		return root_clone;
	}

	/* Otherwise, we need to break the path up and start searching */
	char * path_offset = path;
	uint32_t path_depth = 0;
	while(path_offset < path + path_len) {
		/* Find each PATH_SEPARATOR */
		if(*path_offset == PATH_SEPARATOR) {
			*path_offset = '\0';
			path_depth++;
		}
		path_offset++;
	}
	/* Clean up */
	path[path_len] = '\0';
	path_offset = path + 1;

	/*
	 * At this point, the path is tokenized and path_offset points
	 * to the first token (directory) and path_depth is the number
	 * of directories in the path
	 */

	/*
	 * Dig through the (real) tree to find the file
	 */
	unsigned int depth = 0;
	/* Find the mountpoint for this file */
	FILE * node_ptr = get_mount_point(path, path_depth, &path_offset, &depth);
	if(!node_ptr) return 0;

	if(path_offset >= path + path_len) {
		free(path);
		fopen(node_ptr, flags);
		return node_ptr;
	}

	/* Look for it: */
	FILE * node_next = 0;
	for(; depth < path_depth; ++depth) {
		node_next = fs_finddir(node_ptr, path_offset);
		fclose(node_ptr);
		node_ptr = node_next;
		if(!node_ptr) {
			/* We failed to find the requested directory */
			free((void*)path);
			return 0;
		}

		/*
		 * This test is a little complicated, but we basically always resolve symlinks in the
		 * of a path (like /home/symlink/file) even if O_NOFOLLOW and O_PATH are set. If we are
		 * on the leaf of the path then we will look at those flags and act accordingly
		 */
		if ((node_ptr->flags & FS_SYMLINK) &&
				!((flags & O_NOFOLLOW) && (flags & O_PATH) && depth == path_depth - 1)) {
			/* This ensures we don't return a path when NOFOLLOW is requested but PATH
			 * isn't passed.
			 */
			if ((flags & O_NOFOLLOW) && depth == path_depth - 1) {
				free((void*)path);
				fclose(node_ptr);
				return (FILE*)-1;
			}
			if (symlink_depth >= MAX_SYMLINK_DEPTH) {
				free((void*)path);
				fclose(node_ptr);
				return (FILE*)-2;
			}

			/*
			 * This may actually be big enough that we wouldn't want to allocate it on
			 * the stack, especially considering this function is called recursively
			 */
			char symlink_buf[MAX_SYMLINK_SIZE];
			int len = node_ptr->readlink(node_ptr, symlink_buf, sizeof(symlink_buf));
			if(len < 0) {
				free((void*)path);
				fclose(node_ptr);
				return (FILE*)-3;
			}
			if (symlink_buf[len] != '\0') {
				free((void*)path);
				fclose(node_ptr);
				return (FILE*)-4;
			}

			FILE * old_node_ptr = node_ptr;
			/* Rebuild our path up to this point. This is hella hacky. */
			char * relpath = (char*)malloc(path_len + 1);
			char * ptr = relpath;
			memcpy(relpath, path, path_len + 1);
			for (unsigned int i = 0; i < depth; i++) {
				while(*ptr != '\0')
					ptr++;
				*ptr = PATH_SEPARATOR;
			}
			node_ptr = kopen_recur(symlink_buf, 0, symlink_depth + 1, relpath);
			free(relpath);
			fclose(old_node_ptr);
			if(!node_ptr) {
				free((void*)path);
				return (FILE*)-5;
			}
		}
		if(depth == path_depth - 1) {
			/* We found the file and are done, open the node */
			fopen(node_ptr, flags);
			free((void *)path);
			return node_ptr;
		}
		/* We are still searching... */
		path_offset += strlen(path_offset) + 1;
	}

	/* The file was not found */
	free((void*)path);
	return 0;
}
