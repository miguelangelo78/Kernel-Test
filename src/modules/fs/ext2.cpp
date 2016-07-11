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

/*
 * These macros deal with the block group descriptor bitmap
 */
#define BLOCKBIT(n)  (bg_buffer[((n) >> 3)] & NTH_BIT((((n) % 8))))
#define BLOCKBYTE(n) (bg_buffer[((n) >> 3)])
#define SETBIT(n)    NTH_BIT((((n) % 8)))

#define GETFS(node) (ext2_fs_t*)node->device

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

/*********************************
 ****** FUNCTION PROTOTYPES ******
 *********************************/
static int read_block(ext2_fs_t * fs, unsigned int block_no, uint8_t * buff);
static int write_block(ext2_fs_t * fs, unsigned int block_no, uint8_t *buff);
static ext2_inodetable_t * read_inode(ext2_fs_t * fs, uint32_t inode);
static int write_inode(ext2_fs_t * fs, ext2_inodetable_t * inode, uint32_t index);
static unsigned int ext2_sync(ext2_fs_t * fs);
static int cache_flush_dirty(ext2_fs_t * fs, unsigned int ent_no);
static ext2_dir_t * ext2_direntry(ext2_fs_t * fs, ext2_inodetable_t * inode, uint32_t no, uint32_t index);
static void refresh_inode(ext2_fs_t * fs, ext2_inodetable_t * inodet, uint32_t inode);
static unsigned int inode_read_block(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int block, uint8_t * buff);
static unsigned int inode_write_block(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int inode_no, unsigned int block, uint8_t * buff);
static unsigned int allocate_block(ext2_fs_t * fs);
static unsigned int set_block_number(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int inode_no, unsigned int iblock, unsigned int rblock);


/*********************************************/
/***** EXT2 FILESYSTEM HANDLER FUNCTIONS *****/
/*********************************************/
static uint32_t ext2_open(FILE * node, unsigned int flags) {
	ext2_fs_t * fs = GETFS(node);
	if(flags & O_TRUNC) {
		ext2_inodetable_t * inode = read_inode(fs, node->inode);
		inode->size = 0;
		write_inode(fs, inode, node->inode);
	}
	return 0;
}

static uint32_t ext2_close(FILE * node) {
	/* Nothing to do here */
	return 0;
}

static int ext2_chmod(FILE * node, int mode) {
	ext2_fs_t * fs = GETFS(node);
	ext2_inodetable_t * inode = read_inode(fs, node->inode);
	inode->mode = (inode->mode & 0xFFFFF000) | mode;
	write_inode(fs, inode, node->inode);
	ext2_sync(fs);
	return 0;
}

static struct dirent * ext2_readdir(FILE * node, uint32_t index) {
	ext2_fs_t * fs = GETFS(node);
	ext2_inodetable_t * inode = read_inode(fs, node->inode);
	/* Fetch directory entry: */
	ext2_dir_t * direntry = ext2_direntry(fs, inode, node->inode, index);
	if(!direntry) {
		free(inode);
		return 0;
	}

	/* Cast direntry into dirent struct: */
	struct dirent * dirent = (struct dirent*)malloc(sizeof(struct dirent));
	memcpy(&dirent->name, &direntry->name, direntry->name_len);
	dirent->name[direntry->name_len] = '\0';
	dirent->ino = direntry->inode;
	free(direntry);
	free(inode);
	return dirent;
}

static FILE * ext2_finddir(FILE * node, char * name) {
	return 0;
}

static void ext2_create(FILE * parent, char * name, uint16_t permission) {
	// TODO
}

static void ext2_mkdir(FILE * parent, char * name, uint16_t permission) {
	// TODO
}

static void ext2_unlink(FILE * node, char * name) {
	// TODO
}

/*****************************************/
/***** EXT2 IMPLEMENTATION FUNCTIONS *****/
/*****************************************/

/**
 * ext2->rewrite_superblock Rewrite the superblock.
 *
 * Superblocks are a bit different from other blocks, as they are always in the same place,
 * regardless of what the filesystem block size is. This doesn't work well with our setup,
 * so we need to special-case it.
 */
static int rewrite_superblock(ext2_fs_t * fs) {
	fwrite(fs->block_device, 1024, sizeof(ext2_superblock_t), (uint8_t *)SB);
	return E_SUCCESS;
}

/**
 * ext2->allocate_inode_block Allocate a block in an inode.
 *
 * @param inode Inode to operate on
 * @param inode_no Number of the inode (this is not part of the struct)
 * @param block Block within inode to allocate
 * @returns Error code or E_SUCCESS
 */
static int allocate_inode_block(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int inode_no, unsigned int block) {
	unsigned int block_no = allocate_block(fs);
	if(!block_no) return E_NOSPACE;

	set_block_number(fs, inode, inode_no, block, block_no);
	unsigned int t = (block + 1) * (fs->block_size / 512);
	if(inode->blocks < t) {
		inode->blocks = t;
	}
	write_inode(fs, inode, inode_no);
	return E_SUCCESS;
}

/**
 * ext2->get_block_number Given an inode block number, get the real block number.
 *
 * @param inode   Inode to operate on
 * @param iblock  Block offset within the inode
 * @returns Real block number
 */
static unsigned int get_block_number(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int iblock) {
	// TODO
}

/**
 * ext2->set_block_number Set the "real" block number for a given "inode" block number.
 *
 * @param inode   Inode to operate on
 * @param iblock  Block offset within the inode
 * @param rblock  Real block number
 * @returns Error code or E_SUCCESS
 */
static unsigned int set_block_number(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int inode_no, unsigned int iblock, unsigned int rblock) {
	// TODO
}


static ext2_dir_t * ext2_direntry(ext2_fs_t * fs, ext2_inodetable_t * inode, uint32_t no, uint32_t index) {
	uint8_t * block = (uint8_t*)malloc(fs->block_size);
	uint8_t block_nr = 0;
	inode_read_block(fs, inode, block_nr, block);

	uint32_t dir_offset = 0;
	uint32_t total_offset = 0;
	uint32_t dir_index = 0;

	while (total_offset < inode->size && dir_index <= index) {
		ext2_dir_t *d_ent = (ext2_dir_t *)((uintptr_t)block + dir_offset);
		if(d_ent->inode != 0 && dir_index == index) {
			ext2_dir_t *out = (ext2_dir_t*)malloc(d_ent->rec_len);
			memcpy(out, d_ent, d_ent->rec_len);
			free(block);
			return out;
		}

		dir_offset += d_ent->rec_len;
		total_offset += d_ent->rec_len;

		if(d_ent->inode)
			dir_index++;

		if(dir_offset >= fs->block_size) {
			block_nr++;
			dir_offset -= fs->block_size;
			inode_read_block(fs, inode, block_nr, block);
		}
	}

	free(block);
	return 0;
}

static unsigned int ext2_sync(ext2_fs_t * fs) {
	spin_lock(fs->lock);
	/* Flush each cache entry: */
	for(unsigned int i = 0; i < fs->cache_entries; i++)
		if(DC[i].dirty)
			cache_flush_dirty(fs, i);
	spin_unlock(fs->lock);
	return 0;
}

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

static unsigned int allocate_block(ext2_fs_t * fs) {
	unsigned int block_no = 0;
	unsigned int block_offset = 0;
	unsigned int group = 0;
	uint8_t * bg_buffer = (uint8_t*)malloc(fs->block_size);

	for(unsigned int i = 0; i < fs->block_group_count; i++) {
		if(fs->block_groups[i].free_blocks_count > 0) {
			read_block(fs, fs->block_groups[i].block_bitmap, (uint8_t*)bg_buffer);
			while(BLOCKBIT(block_offset))
				block_offset++;
			block_no = block_offset + SB->blocks_per_group * i;
			group = i;
			break;
		}
	}

	if(!block_no) {
		kprintf("\n\t> ERROR: No available blocks. Disk is out of space!");
		free(bg_buffer);
		return 0;
	}

	BLOCKBYTE(block_offset) |= SETBIT(block_offset);
	write_block(fs, fs->block_groups[group].block_bitmap, (uint8_t*)bg_buffer);

	fs->block_groups[group].free_blocks_count--;
	for(int i = 0; i <fs->bgd_block_span; i++)
		write_block(fs, fs->bgd_offset + i, (uint8_t *)((uint32_t)fs->block_groups + fs->block_size * i));

	SB->free_blocks_count--;
	rewrite_superblock(fs);

	memset(bg_buffer, 0, fs->block_size);
	write_block(fs, block_no, bg_buffer);

	free(bg_buffer);
	return block_no;
}

/**
 * ext2->inode_read_block
 *
 * @param inode
 * @param no
 * @param block
 * @parma buf
 * @returns Real block number for reference.
 */
static unsigned int inode_read_block(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int block, uint8_t * buff) {
	if(block >= inode->blocks / (fs->block_size / 512)) {
		memset(buff, 0, fs->block_size);
		kprintf("\n\tERROR: Tried to read an invalid block. Asked for %d but inode only has %d!", block, inode->blocks / (fs->block_size / 512));
		return 0;
	}
	unsigned int real_block = get_block_number(fs, inode, block);
	read_block(fs, real_block, buff);
	return real_block;
}

/**
 * ext2->inode_write_block
 */
static unsigned int inode_write_block(ext2_fs_t * fs, ext2_inodetable_t * inode, unsigned int inode_no, unsigned int block, uint8_t * buff) {
	if(block >= inode->blocks / (fs->block_size / 512)) {
		kprintf("\n\tERROR: Attempting to write beyond the existing allocated blocks for this inode");
		kprintf(" (Inode %d, Block %d)", inode_no, block);
	}

	char * empty = 0;
	while(block >= inode->blocks / (fs->block_size / 512)) {
		allocate_inode_block(fs, inode, inode_no, inode->blocks / (fs->block_size / 512));
		refresh_inode(fs, inode, inode_no);
	}

	if(empty) free(empty);

	unsigned int real_block = get_block_number(fs, inode, block);
	write_block(fs, real_block, buff);
	return real_block;
}

static void refresh_inode(ext2_fs_t * fs, ext2_inodetable_t * inodet, uint32_t inode) {
	uint32_t group = inode / fs->inodes_per_group;
	if(group > fs->block_group_count)
		return;

	/* Calculate inode location: */
	uint32_t inode_table_block = fs->block_groups[group].inode_table;
	inode -= group * fs->inodes_per_group; /* Adjust index within group */
	uint32_t block_offset = ((inode - 1) * fs->inode_size) / fs->block_size;
	uint32_t offset_in_block = (inode - 1) - block_offset * (fs->block_size / fs->inode_size);

	/* Read the inode (from ATA) and cast into inode structure: */
	uint8_t * buff = (uint8_t*)malloc(fs->block_size);
	read_block(fs, inode_table_block + block_offset, buff);

	/* Prepare inodet: */
	ext2_inodetable_t * inodes = (ext2_inodetable_t*)buff;
	memcpy(inodet, (uint8_t*)((uint32_t)inodes + offset_in_block * fs->inode_size), fs->inode_size);
	free(buff);
}

static ext2_inodetable_t * read_inode(ext2_fs_t * fs, uint32_t inode) {
	ext2_inodetable_t * inodet = (ext2_inodetable_t*)malloc(fs->inode_size);
	refresh_inode(fs, inodet, inode);
	return inodet;
}

static int write_inode(ext2_fs_t * fs, ext2_inodetable_t * inode, uint32_t index) {
	uint32_t group = index / fs->inodes_per_group;
	if(group > fs->block_group_count)
		return E_BADBLOCK;

	uint32_t inode_table_block = fs->block_groups[group].inode_table;
	index -= group * fs->inodes_per_group;
	uint32_t block_offset = ((index - 1) * fs->inode_size) / fs->block_size;
	uint32_t offset_in_block = (index - 1) - block_offset * (fs->block_size / fs->inode_size);

	ext2_inodetable_t * inodet = (ext2_inodetable_t*)malloc(fs->block_size);
	/* Read the current table block: */
	read_block(fs, inode_table_block + block_offset, (uint8_t*)inodet);
	memcpy((uint8_t *)((uint32_t)inodet + offset_in_block * fs->inode_size), inode, fs->inode_size);
	write_block(fs, inode_table_block + block_offset, (uint8_t*)inodet);
	free(inodet);
	return E_SUCCESS;
}

/**
 * ext2->write_block Write a block to the block device.
 *
 * @param block_no Block to write
 * @param buf      Data in the block
 * @returns Error code or E_SUCCESSS
 */
static int write_block(ext2_fs_t * fs, unsigned int block_no, uint8_t *buff) {
	if(block_no == 0) {
		kprintf("\n\t> ERROR: Attempted to write to block #0. Aborting");
		return E_BADBLOCK;
	}

	spin_lock(fs->lock);
	if(!DC) {
		/* Write to block device (such as ATA): */
		fwrite(fs->block_device, block_no * fs->block_size, fs->block_size, buff);
		spin_unlock(fs->lock);
		return E_SUCCESS;
	}

	/* Else: Find the entry in the cache: */
	int oldest = -1;
	unsigned int oldest_age = UINT32_MAX;
	for(unsigned int i = 0; i <fs->cache_entries; i++) {
		if(DC[i].block_no == block_no) {
			/* We found it. Update the cache entry */
			DC[i].last_use = get_cache_time(fs);
			DC[i].dirty = 1;
			memcpy(DC[i].block, buff, fs->block_size);
			spin_unlock(fs->lock);
			return E_SUCCESS;
		}
		if(DC[i].last_use < oldest_age) {
			/* Keep track of the oldest entry */
			oldest = i;
			oldest_age = DC[i].last_use;
		}
	}

	/* We did not find this element in the cache, so make room */
	if(DC[oldest].dirty)
		/* Flush the oldest entry */
		cache_flush_dirty(fs, oldest);

	/* Update the entry: */
	memcpy(DC[oldest].block, buff, fs->block_size);
	DC[oldest].block_no = block_no;
	DC[oldest].last_use = get_cache_time(fs);
	DC[oldest].dirty = 1;

	/* Done: */
	spin_unlock(fs->lock);
	return E_SUCCESS;
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
	if ((inode->mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
		kprintf("\n\t> ERROR: Root appears to be a normal file. Aborting");
		return 0;
	}
	if ((inode->mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
		/* It's a directory, which is valid */
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
		kprintf("(VALID EXT2 DETECTED)");
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
