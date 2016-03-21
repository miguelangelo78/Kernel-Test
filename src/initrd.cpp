/*
 * initrd.cpp
 *
 *  Created on: 20/03/2016
 *      Author: Miguel
 */

#include <system.h>

initrd_header_t * initrd_header;

FILE * initrd_root;
FILE * root_files;

struct dirent dirent;

unsigned int * fs_getfile_addr(FILE * node) {
	return (unsigned int*)((uintptr_t)(&initrd_header->header_size) + ((unsigned int*)(&initrd_header->offset))[node->inode]);
}

static uint32_t initrd_read(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	if(offset > node->size) offset = 0;
	if(size > node->size) size = node->size;

	memcpy(buffer, fs_getfile_addr(node) + offset, size);
	return size;
}

static struct dirent * initrd_readdir(FILE * node, uint32_t index) {
	if (index > initrd_header->file_count) return 0;

	strcpy(dirent.name, root_files[index].name);
	dirent.name[strlen(root_files[index].name)] = 0; // Make sure the string is NULL-terminated.
	dirent.ino = root_files[index].inode;
	return &dirent;
}

static FILE * initrd_finddir(FILE * node, char * name) {
	for (uint32_t i = 0; i < initrd_header->file_count; i++)
		if (!strcmp(name, root_files[i].name))
			return &root_files[i];
	return 0;
}

#define ALIGN_LENGTH(length_index) ((initrd_header->file_count - 1) + length_index)
#define ALIGN_FILENAME() (ALIGN_LENGTH(initrd_header->file_count-1)*sizeof(unsigned int))

FILE * initrd_init(uint32_t location) {
	initrd_header = (initrd_header_t*)location;

	kprintf("!Found %d files! ", initrd_header->file_count);

	/* Init root: */
	initrd_root = (FILE*)kmalloc(sizeof(FILE));
	strcpy(initrd_root->name, "initrd");
	initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->size = 0;
	initrd_root->flags = FS_DIR;
	initrd_root->read = 0;
	initrd_root->write = 0;
	initrd_root->open = 0;
	initrd_root->close = 0;

	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;

	initrd_root->ptr = 0;

	root_files = (FILE*)kmalloc(sizeof(FILE) * initrd_header->file_count);

	char * filename_ptr = (char*)((uint32_t)(&initrd_header->filename) + ALIGN_FILENAME());
	for(unsigned int i = 0; i < initrd_header->file_count; i++) {
		strcpy(root_files[i].name, filename_ptr);

		root_files[i].mask = root_files[i].uid = root_files[i].gid = 0;
		root_files[i].size = (uint32_t)(*(((unsigned int*)&initrd_header->length) + ALIGN_LENGTH(i)));
		root_files[i].inode = i;
		root_files[i].flags = FS_FILE;
		root_files[i].read = &initrd_read;
		root_files[i].write = 0;
		root_files[i].readdir = 0;
		root_files[i].finddir = 0;
		root_files[i].open = 0;
		root_files[i].close = 0;

		if(Log::logging == LOG_SERIAL || 1) {
			kprintf("\n   * Module %d (@0x%x > @0x%x): %s", i+1,
					(uintptr_t)(&initrd_header->header_size) + ((unsigned int*)(&initrd_header->offset))[i],
					root_files[i].size, filename_ptr);
		}
		filename_ptr += INITRD_FILENAME_SIZE;
	}

	if(Log::logging == LOG_SERIAL) { kprintf("\n"); }
	return initrd_root;
}

char * initrd_getmod(char * modname) {
	FILE * node = fs_finddir(root, modname);
	if(!node) return 0;

	if(node->size > 1) {
		char * buff = (char*)malloc(node->size+1);
		fread(node, 0, node->size, (unsigned char*)buff);
		buff[node->size] = 0;
		return buff;
	} else {
		return 0;
	}
}

char * initrd_getmod(int mod_id) {
	return mod_id < initrd_modcount() ? initrd_getmod(initrd_readdir(root, mod_id)->name) : 0;
}

int initrd_modcount(void) {
	return initrd_header->file_count;
}
