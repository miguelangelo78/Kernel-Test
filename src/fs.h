#pragma once

#include <stdint.h>

/***********************************************************/
/************* VFS Macros / struct definitions *************/
/***********************************************************/
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP ".."
#define PATH_DOT "."

#define O_RDONLY   0x0000
#define O_WRONLY   0x0001
#define O_RDWR     0x0002
#define O_APPEND   0x0008
#define O_CREAT    0x0200
#define O_TRUNC    0x0400
#define O_EXCL     0x0800
#define O_NOFOLLOW 0x1000
#define O_PATH     0x2000

enum FS_FLAGS {
	FS_FILE       = 0x01,
	FS_DIR        = 0x02,
	FS_CHARDEV    = 0x04,
	FS_BLOCKDEV   = 0x08,
	FS_PIPE       = 0x10,
	FS_SYMLINK    = 0x20,
	FS_MOUNTPOINT = 0x40
};

#define _IFMT   0170000 /* type of file */
#define _IFDIR  0040000 /* directory */
#define _IFCHR  0020000 /* character special */
#define _IFBLK  0060000 /* block special */
#define _IFREG  0100000 /* regular */
#define _IFLNK  0120000 /* symbolic link */
#define _IFSOCK 0140000 /* socket */
#define _IFIFO  0010000 /* fifo */



/************************/
/**** FILE structure ****/
/************************/
typedef struct fs_node {
	char name[128];      /* The filename */
	void * device;       /* Device object (optional) */
	uint32_t mask;       /* The permissions mask */
	uint32_t uid;        /* The owning user */
	uint32_t gid;        /* The owning group */
	uint32_t flags;      /* Flags (node type, etc) */
	uint32_t inode;      /* Inode number */
	uint32_t size;       /* Size of the file, in bytes */
	uint32_t impl;       /* Used to keep track which filesystem it belongs to */
	uint32_t open_flags; /* Flags passed to open (read/write/append, etc.) */

	uint32_t atime; /* Accessed time */
	uint32_t mtime; /* Modified time */
	uint32_t ctime; /* Created time */

	struct fs_node * ptr; /* Alias pointer, for symlinks. */
	uint32_t offset;
	int32_t refcount;
	uint32_t nlink;

	/*********************** Callbacks/file operations**************/
	uint32_t (*read)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
	uint32_t (*write)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
	uint32_t (*open)(struct fs_node*, unsigned int flags);
	uint32_t (*close)(struct fs_node*);
	struct dirent * (*readdir)(struct fs_node*, uint32_t);
	struct fs_node *(*finddir)(struct fs_node*, char * name);
	void (*create) (struct fs_node *, char *name, uint16_t permission);
	void (*mkdir) (struct fs_node *, char *name, uint16_t permission);
	int (*ioctl) (struct fs_node *, int request, void * argp);
	int (*get_size) (struct fs_node *);
	int (*chmod) (struct fs_node *, int mode);
	void (*unlink) (struct fs_node *, char *name);
	void (*symlink) (struct fs_node *, char * name, char * value);
	int (*readlink) (struct fs_node *, char * buf, size_t size);
} FILE;

/** Directory entry **/
struct dirent {
	char name[128]; /* Directory name */
	uint32_t ino;   /* INode number   */
};

/** Struct that holds packets of file statistics/information **/
struct stat {
	uint16_t st_dev;
	uint16_t st_ino;
	uint32_t st_mode;
	uint16_t st_nlink;
	uint16_t st_uid;
	uint16_t st_gid;
	uint16_t st_rdev;
	uint32_t st_size;
	uint32_t st_atime;
	uint32_t __unused1;
	uint32_t st_mtime;
	uint32_t __unused2;
	uint32_t st_ctime;
	uint32_t __unused3;
};

struct vfs_entry {
	char * name;
	FILE * file;
};


/****************************************************************/
/************* VFS Installers/Initializers/Mounters *************/
/****************************************************************/
extern void vfs_install(void);
extern void vfs_install(uintptr_t initrd_location); /* Install with Initrd's location */

typedef FILE * (*vfs_mount_callback)(char * arg, char * mount_point);

extern void * vfs_mount(char * path, FILE * local_root);
extern int vfs_register(char * filesystem_name, vfs_mount_callback * cback);
extern int vfs_mount_type(char * type, char * arg, char * mountpoint);
extern void vfs_lock(FILE * node);

extern void map_vfs_directory(char *);
extern int make_unix_pipe(FILE ** pipes);


/*****************************************/
/************* VFS Functions *************/
/*****************************************/
extern uint32_t fread(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer);
extern uint32_t fwrite(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer);
extern uint32_t fopen(FILE * node, unsigned int flags);
extern uint32_t fclose(FILE * node);
extern struct dirent * fs_readdir(FILE * node, uint32_t index);
extern FILE * fs_finddir(FILE * node, char * name);
extern uint32_t fs_filesize(FILE * node);
extern char fs_is_dir(FILE * node);
extern int fs_mkdir(char * dirname, uint16_t permission);
extern int fs_create_file(char * filename, uint16_t permission);
extern FILE * kopen(char * filename, uint32_t flags);
extern char * canonicalize_path(char * cwd, char * input);
extern FILE * fs_clone(FILE * source);
extern int fs_ioctl(FILE * node, int request, void * argp);
extern int fs_chmod(FILE * node, int mode);
extern int fs_unlink(char * filename);
extern int fs_symlink(char * target, char * filename);
extern int fs_readlink(FILE * node, char * buff, size_t size);
extern int pty_create(void * size, FILE ** fs_master, FILE ** fs_slave);
extern char * canonicalize_path(char * cwd, char * input);

/** Virtual Filesystem Root: **/
extern FILE * fs_root;
