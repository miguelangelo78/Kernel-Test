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

/****************************
 ****** MACROS / ENUMS ******
 ****************************/
#define USE_CACHE 0

#define SB (fs->superblock)
#define DC (fs->disk_cache)

enum E_CODES {
	E_SUCCESS,
	E_BADBLOCK,
	E_NOSPACE,
	E_BADPARENT
};

/*************************
 * EXT2 Filesystem Object
 *************************/
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

	uint8_t                   bgd_block_span;
	uint8_t                   bgd_offset;
	unsigned int              inode_size;
} ext2_fs_t;

/*********************************************/
/***** EXT2 FILESYSTEM HANDLER FUNCTIONS *****/
/*********************************************/
static uint32_t ext2_open(FILE * node, unsigned int flags) {
	return 0;
}

static uint32_t ext2_close(FILE * node) {
	return 0;
}

static int ext2_chmod(FILE * node, int mode) {
	return 0;
}

static struct dirent * ext2_readdir(FILE * node, uint32_t index) {
	return 0;
}

static FILE * ext2_finddir(FILE * node, char * name) {
	return 0;
}

static void ext2_create(FILE * parent, char * name, uint16_t permission) {

}

static void ext2_mkdir(FILE * parent, char * name, uint16_t permission) {

}

static void ext2_unlink(FILE * node, char * name) {

}

/*****************************************/
/***** EXT2 IMPLEMENTATION FUNCTIONS *****/
/*****************************************/

/****** CACHE FUNCTIONS ******/
/**
 * ext2->get_cache_time Increment and return the current cache time
 *
 * @returns Current cache time
 */
static unsigned int get_cache_time(ext2_fs_t * fs) {
	return fs->cache_time++;
}

/**
 * ext2->cache_flush_dirty Flush dirty cache entry to the disk.
 *
 * @param ent_no Cache entry to dump
 * @returns Error code or E_SUCCESS
 */
static int cache_flush_dirty(ext2_fs_t * fs, unsigned int ent_no) {
	fwrite(fs->block_device, (DC[ent_no].block_no) * fs->block_size, fs->block_size, (uint8_t *)(DC[ent_no].block));
	DC[ent_no].dirty = 0;
	return E_SUCCESS;
}

/****** BLOCK / INODE READERS / WRITERS ******/
static ext2_inodetable_t * read_inode(ext2_fs_t * fs, uint32_t inode) {

	return 0;
}

/**
 * ext2->read_block Read a block from the block device associated with this filesystem.
 *
 * The read block will be copied into the buffer pointed to by `buff`.
 *
 * @param block_no Number of block to read.
 * @param buf      Where to put the data read.
 * @returns Error code or E_SUCCESS
 */
static int read_block(ext2_fs_t * fs, unsigned int block_no, uint8_t * buff) {
	/* 0 is an invalid block number. So is anything beyond the total block count, but we can't check that. */
	if(!block_no) return E_BADBLOCK;

	/* This operation requires the filesystem lock to be obtained */
	spin_lock(fs->lock);

	/* Read directory from the block device (e.g.: ATA): */
	if(!DC) {
		fread(fs->block_device, block_no * fs->block_size, fs->block_size, (uint8_t*)buff);
		/* Done reading: */
		spin_unlock(fs->lock);
		return E_SUCCESS;
	}
	/* Otherwise read from cache */

	/*
	 * Search the cache for this entry
	 * We'll look for the oldest entry, too.
	 */
	int oldest = -1;
	unsigned int oldest_age = UINT32_MAX;
	for(unsigned int i = 0; i < fs->cache_entries; i++) {
		if(DC[i].block_no == block_no) {
			/* We found it! Update usage times */
			DC[i].last_use = get_cache_time(fs);
			/* Read the block: */
			memcpy(buff, DC[i].block, fs->block_size);
			/* Done reading: */
			spin_unlock(fs->lock);
			return E_SUCCESS;
		}
		if(DC[i].last_use < oldest_age) {
			/* We found an older block, remember this */
			oldest = i;
			oldest_age = DC[i].last_use;
		}
	}

	/*
	 * At this point, we did not find this block in the cache.
	 * We are going to replace the oldest entry with this new one.
	 */

	/* We'll start by flushing the block if it was dirty */
	if(DC[oldest].dirty)
		cache_flush_dirty(fs, oldest);

	/* Then we'll read the new one */
	fread(fs->block_device, block_no * fs->block_size, fs->block_size, (uint8_t *)DC[oldest].block);

	/* And copy the results to the output buffer */
	memcpy(buff, DC[oldest].block, fs->block_size);

	/* And update the cache entry to point to the new block */
	DC[oldest].block_no = block_no;
	DC[oldest].last_use = get_cache_time(fs);
	DC[oldest].dirty = 0;

	/* Done reading: */
	spin_unlock(fs->lock);
	return E_SUCCESS;
}

/*******************************************/
/***** EXT2 MOUNTERS / FS INITIALIZERS *****/
/*******************************************/
static uint32_t ext2_make_rootfile(ext2_fs_t * fs, ext2_inodetable_t * inode, FILE * file_node) {
	if(!file_node) return 0;

	/* Information for root dir: */
	file_node->device = (void*)fs;
	file_node->inode = 2;
	file_node->name[0] = '/';
	file_node->name[1] = '\0';

	/* Information from the inode: */
	file_node->uid = inode->uid;
	file_node->gid = inode->gid;
	file_node->size = inode->size;
	file_node->mask = inode->mode & 0xFFF;
	file_node->nlink = inode->links_count;

	/* File flags: */
	file_node->flags = 0;
	if((inode->mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
		kprintf("\n\t> ERROR: Root appears to be a normal file. Aborting");
		return 0;
	}
	if ((inode->mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
		/* It's a directory */
	} else {
		kprintf("\n\t> ERROR: Root doesn't appear to be a directory. Aborting");
		return 0;
	}
	/* Other flags: */
	if ((inode->mode & EXT2_S_IFBLK) == EXT2_S_IFBLK)
		file_node->flags |= FS_BLOCKDEV;
	if ((inode->mode & EXT2_S_IFCHR) == EXT2_S_IFCHR)
		file_node->flags |= FS_CHARDEV;
	if ((inode->mode & EXT2_S_IFIFO) == EXT2_S_IFIFO)
		file_node->flags |= FS_PIPE;
	if ((inode->mode & EXT2_S_IFLNK) == EXT2_S_IFLNK)
		file_node->flags |= FS_SYMLINK;

	/* Creation/Alteration Times: */
	file_node->atime = inode->atime;
	file_node->mtime = inode->mtime;
	file_node->ctime = inode->ctime;

	file_node->flags |= FS_DIR;
	file_node->read = 0;
	file_node->write = 0;
	file_node->chmod = ext2_chmod;
	file_node->open = ext2_open;
	file_node->close = ext2_close;
	file_node->readdir = ext2_readdir;
	file_node->finddir = ext2_finddir;
	file_node->ioctl = 0;
	file_node->create = ext2_create;
	file_node->mkdir = ext2_mkdir;
	file_node->unlink = ext2_unlink;
	return 1;
}

static FILE * mount_ext2(FILE * blockdev) {
	ext2_fs_t * fs = (ext2_fs_t*)malloc(sizeof(ext2_fs_t));
	memset(fs, 0, sizeof(ext2_fs_t));
	fs->block_device = blockdev;
	fs->block_size = 1024;
	vfs_lock(fs->block_device);

	SB = (ext2_superblock_t*)malloc(fs->block_size);

	/* Read/parse superblock (first block): */
	read_block(fs, 1, (uint8_t*)SB);

	if(SB->magic != EXT2_SUPER_MAGIC) {
		kprintf("\n\t> ERROR: NOT a filesystem! (magic did not match, got 0x%x)", SB->magic);
		return 0;
	} else {
		kprintf("(VALID EXT2)");
	}

	fs->inode_size = SB->inode_size;
	if(SB->inode_size == 0)
		fs->inode_size = 128;

	fs->block_size = 1024 << SB->log_block_size;
	fs->cache_entries = 10240;
	if(fs->block_size > 2048)
		fs->cache_entries /= 4;

	fs->pointer_per_block = fs->block_size / 4;

	fs->block_group_count = SB->blocks_count / SB->blocks_per_group;
	if(SB->blocks_per_group * fs->block_group_count < SB->blocks_count)
		fs->block_group_count++;

	fs->inodes_per_group = SB->inodes_count / fs->block_group_count;

#if USE_CACHE == 1
	/* Allocating cache: */
	DC = (ext2_disk_cache_entry_t*)malloc(sizeof(ext2_disk_cache_entry_t));

	uint32_t cache_size = fs->block_size * fs->cache_entries;
	kprintf("\n\t> Allocating Disk Cache (Size: %d MB/ %d KB / %d)\n", (cache_size / 1000) / 1000, cache_size / 1000, cache_size);
	fs->cache_data = (uint8_t*)malloc(cache_size);
	memset(fs->cache_data, 0, cache_size);
	for(uint32_t i = 0; i < fs->cache_entries; i++) {
		DC[i].block_no = 0;
		DC[i].dirty = 0;
		DC[i].last_use = 0;
		DC[i].block = fs->cache_data + i * fs->block_size;
	}
#endif

	/* Load Block Group Descriptors: */
	fs->bgd_block_span = sizeof(ext2_bgdescriptor_t) * fs->block_group_count / fs->block_size + 1;
	fs->block_groups = (ext2_bgdescriptor_t*)malloc(fs->block_size * fs->bgd_block_span);

	fs->bgd_offset = 2;

	if(fs->block_size > 1024)
		fs->bgd_offset = 1;

	for(int i = 0;i < fs->bgd_block_span; i++)
		read_block(fs, fs->bgd_offset + i, (uint8_t*)((uint32_t)fs->block_groups + fs->block_size * i));

	ext2_inodetable_t * root_inode = read_inode(fs, 2);
	fs->root_node = (FILE*)malloc(sizeof(FILE));
	if(!ext2_make_rootfile(fs, root_inode, fs->root_node))
		return 0;
	/* Success */
	return fs->root_node;
}

/* EXT2 mount callback registered by 'vfs_register' and called by 'vfs_mount_type': */
static FILE * ext2_fs_mount(char * devpath, char * mountpath) {
	FILE * device = kopen(devpath, 0); /* Open block device, for example ATA */
	if(!device) {
		kprintf("\n\t> !ERROR!: Could not open '%s'", devpath);
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
