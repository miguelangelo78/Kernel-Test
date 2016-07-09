/*
 * ext2.cpp
 *
 *  Created on: 02/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include "ext2.h"
#include <fs.h>

/*
 * EXT2 Filesystem Object
 */
typedef struct ext2_fs {
	/***** SUPERBLOCK AND BLOCK GROUPS *****/
	ext2_superblock_t       * superblock;        /* Device superblock, contains important information */
	ext2_bgdescriptor_t     * block_groups;      /* Block Group Descriptor / Block groups */
	FILE                    * root_node;         /* Root FS node (attached to mountpoint) */
	FILE                    * block_device;      /* Block device node */

	/***** BLOCKS *****/
	unsigned int              block_size;        /* Size of one block */
	unsigned int              pointer_per_block; /* Number of pointers that fit in a block */
	unsigned int              inodes_per_group;  /* Number of inodes in a "group" */
	unsigned int              block_group_count; /* Number of blocks groups */

	/***** CACHE *****/
	ext2_disk_cache_entry_t * disk_cache;        /* Dynamically allocated array of cache entries */
	unsigned int              cache_entries;     /* Size of ->disk_cache */
	unsigned int              cache_time;        /* "timer" that increments with each cache read/write */
	uint8_t                 * cache_data;

	spin_lock_t               lock;              /* Synchronization lock point */

	uint8_t                   bgd_lock_span;
	uint8_t                   bgd_offset;
	unsigned int              inode_size;
} ext2_fs_t;

static FILE * mount_ext2(FILE * blockdev) {
	ext2_fs_t * fs = (ext2_fs_t*)malloc(sizeof(ext2_fs_t));
	memset(fs, 0, sizeof(ext2_fs_t));
	fs->block_device = blockdev;
	fs->block_size = 1024;
	vfs_lock(fs->block_device);

	fs->superblock = (ext2_superblock_t*)malloc(fs->block_size);

	/* Reading superblock: */

	fs->root_node = (FILE*)malloc(sizeof(FILE));
	return fs->root_node;
}

/* EXT2 mount callback registered by 'vfs_register' and called by 'vfs_mount_type': */
static FILE * ext2_fs_mount(char * devpath, char * mountpath) {
	FILE * device = kopen(devpath, 0); /* Open block device, for example ATA */
	if(!device) {
		kprintf("\n> !ERROR!: Could not open '%s'", devpath);
		return 0;
	}
	/* Return ext2 filesystem: */
	return mount_ext2(device);
}

/****************************************************/
/***** MODULE INITIALIZERS/DEINITIALIZERS/IOCTL *****/
/****************************************************/
static int ext2_init(void) {
	vfs_register((char*)"ext2", (vfs_mount_callback)ext2_fs_mount);
	return 0;
}

static int ext2_finit(void) {
	return 0;
}

static uintptr_t ext2_ioctl(void * ioctl_packet) {
	return 0;
}

MODULE_DEF(ext2_driver, ext2_init, ext2_finit, MODT_FS, "Miguel S.", ext2_ioctl);
