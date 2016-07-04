#include <fs.h>
#include <libc/tree.h>
#include <libc/list.h>
#include <libc/hashmap.h>
#include <system.h>

/***********************************************************/
/************* VFS Macros / definitions ********************/
/***********************************************************/
#define MAX_SYMLINK_DEPTH 8
#define MAX_SYMLINK_SIZE 4096

tree_t * fs_tree = 0; /* Filesystem mountpoint tree */
hashmap_t * fs_types = 0;
FILE * fs_root = 0; /* Pointer to the root mount fs_node (must be some form of filesystem, even ramdisk) */


/***********************************************/
/************* Function Prototypes *************/
/***********************************************/
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
	vfs_mount("todo:temporary", initrd_init(*(uint32_t*)initrd_location));

}

void * vfs_mount(char * path, FILE * local_root) {
	fs_root = local_root;

	return 0;
}

/* Register new filesystem in the VFS: */
int vfs_register(char * filesystem_name, vfs_mount_callback * cback) {
	if(hashmap_get(fs_types, filesystem_name)) return 1;
	hashmap_set(fs_types, filesystem_name, (void*)(uintptr_t)(cback));
	return 0;
}

int vfs_mount_type(char * type, char * arg, char * mountpoint) {

	return 0;
}

static spin_lock_t tmp_refcount_lock = { 0 };

void vfs_lock(FILE * node) {
	spin_lock(tmp_refcount_lock);
	// TODO
	spin_unlock(tmp_refcount_lock);
}

void map_vfs_directory(char * c) {
	FILE * f = vfs_mapper();
	struct vfs_entry * e = (struct vfs_entry *)vfs_mount(c, f);
	// TODO:
}

int make_unix_pipe(FILE ** pipes) {

	return 0;
}


/*****************************************/
/************* VFS Functions *************/
/*****************************************/
char fs_is_dir(FILE * node) {
	/* Is node a directory: */
	return (node->flags & 0x7) == FS_DIR;
}

uint32_t fread(FILE * node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	return node && node->read ? node->read(node, offset, size, buffer) : -1;
}

uint32_t fwrite(FILE * node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	return node && node->write ? node->write(node, offset, size, buffer) : -1;
}

uint32_t fopen(FILE * node, uint8_t read, uint8_t write) {
	return node && node->open ? node->open(node) : -1;
}

uint32_t fclose(FILE * node) {
	return node && node->close ? node->close(node) : -1;
}

struct dirent * fs_readdir(FILE* node, uint32_t index) {
	return (node && fs_is_dir(node) && node->readdir) ? node->readdir(node, index) : 0;
}

FILE * fs_finddir(FILE* node, char * name) {
	return (node && fs_is_dir(node) && node->finddir) ? node->finddir(node, name) : 0;
}

uint32_t fs_filesize(FILE * node) {
	return node->size;
}

int fs_mkdir(char * dirname, uint16_t permission) {
	return 0;
}

int fs_create_file(char * filename, uint16_t permission) {
	return 0;
}

FILE * kopen(char * filename, uint32_t flags) {
	return kopen_recur(filename, flags, 0, 0); // TODO
}

char * canonicalize_path(char * cwd, char * input) {
	return 0;
}

FILE * fs_clone(FILE * source) {
	return 0;
}

int fs_ioctl(FILE * node, int request, void * argp) {
	return 0;
}

int fs_chmod(FILE * node, int mode) {
	return 0;
}

int fs_unlink(char * filename) {
	return 0;
}

int fs_symlink(char * value, char * filename) {
	return 0;
}

int fs_readlink(FILE * node, char * buff, size_t size) {
	return 0;
}

int pty_create(void * size, FILE ** fs_master, FILE ** fs_slave) {
	return 0;
}


/********************************************************/
/************* VFS Implementation functions *************/
/********************************************************/
static struct dirent * readdir_mapper(FILE *node, uint32_t index) {

}

static FILE * vfs_mapper(void) {

}

FILE * get_mount_point(char * path, unsigned int path_depth, char **outpath, unsigned int * outdepth) {

}

FILE * kopen_recur(char *filename, uint32_t flags, uint32_t symlink_depth, char * relative_to) {

}
