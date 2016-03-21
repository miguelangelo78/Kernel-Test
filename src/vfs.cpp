#include <fs.h>
#include <libc\tree.h>
#include <libc\list.h>
#include <libc\hashmap.h>
#include <system.h>
#include <process.h>

FILE * root = 0;

/* Is node a directory: */
char fs_is_dir(FILE * node) {
	return (node->flags & 0x7) == FS_DIR;
}

void vfs_install(void) {

}

void vfs_install(uintptr_t initrd_location) {
	root = initrd_init(*(uint32_t*)initrd_location);
}

uint32_t fread(FILE* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	return node->read ? node->read(node, offset, size, buffer) : 0;
}

uint32_t fs_write(FILE* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	return node->write ? node->write(node, offset, size, buffer) : 0;
}

uint32_t fs_open(FILE* node, uint8_t read, uint8_t write) {
	return node->open ? node->open(node) : 0;
}

uint32_t fs_close(FILE* node) {
	return node->close ? node->close(node) : 0;
}

struct dirent * fs_readdir(FILE* node, uint32_t index) {
	return (fs_is_dir(node) && node->readdir) ? node->readdir(node, index) : 0;
}

struct fs_node * fs_finddir(FILE* node, char * name) {
	return (fs_is_dir(node) && node->finddir) ? node->finddir(node, name) : 0;
}

uint32_t fs_filesize(FILE * node) {
	return node->size;
}
