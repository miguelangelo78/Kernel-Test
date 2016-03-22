/*
 * initrd.h
 *
 *  Created on: 20/03/2016
 *      Author: Miguel
 */

#ifndef SRC_INITRD_H_
#define SRC_INITRD_H_

#include <stdint.h>
#include <fs.h>

typedef struct {
	unsigned int header_size;
	unsigned int file_count;
	unsigned int offset, length;
	char filename;
} __packed initrd_header_t;

#define INITRD_FILENAME_SIZE 64

extern FILE * initrd_init(uint32_t location);

/* These functions are dedicated for initrd. They "bypass" the VFS, but the VFS still works if you want to use the initrd */
extern char * initrd_readfile(FILE * file, char);
extern char * initrd_readfile(FILE * file);
extern char * initrd_readfile(char *);
extern char * initrd_readfile(int file_id);
extern char * initrd_getmod(char * modname);
extern char * initrd_getmod(int mod_id);
extern char * initrd_getmod_name(int mod_id);
extern int initrd_modcount(void);
extern int initrd_filecount(void);
extern FILE * initrd_getfile(char * filename);
extern FILE * initrd_getfile(int file_id);
extern FILE * initrd_getmod_file(char * modname);
extern FILE * initrd_getmod_file(int mod_id);

#endif /* SRC_INITRD_H_ */
