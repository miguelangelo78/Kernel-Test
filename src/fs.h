#pragma once

#include <stdint.h>

enum FS_FLAGS {
	FS_FILE = 0x1,
	FS_DIR = 0x2,
	FS_CHARDEV = 0x3,
	FS_BLOCKDEV = 0x4,
	FS_PIPE = 0x5,
	FS_SYMLINK = 0x6,
	FS_MOUNTPOINT = 0x8
};

struct dirent {
	char name[128];
	uint32_t ino;
};

typedef struct fs_node {
	char name[128];
	uint32_t mask;
	uint32_t uid;
	uint32_t gid;
	uint32_t flags;
	uint32_t inode;
	uint32_t size;
	struct fs_node * ptr;
	uint32_t (*read)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
	uint32_t (*write)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
	uint32_t (*open)(struct fs_node*);
	uint32_t (*close)(struct fs_node*);
	struct dirent * (*readdir)(struct fs_node*, uint32_t);
	struct fs_node *(*finddir)(struct fs_node*, char * name);
} FILE;

extern char fs_is_dir(FILE * node);

extern uint32_t fread(FILE* node, uint32_t offset, uint32_t size, uint8_t* buffer);
extern uint32_t fs_write(FILE* node, uint32_t offset, uint32_t size, uint8_t* buffer);
extern uint32_t fs_open(FILE* node, uint8_t read, uint8_t write);
extern uint32_t fs_close(FILE* node);
extern struct dirent * fs_readdir(FILE* node, uint32_t index);
extern struct fs_node *fs_finddir(FILE* node, char * name);
extern uint32_t fs_filesize(FILE * node);

extern void vfs_install(void);
extern void vfs_install(uintptr_t initrd_location); /* Install with Initrd's location */

extern FILE * root;

