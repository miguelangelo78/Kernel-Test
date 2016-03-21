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
extern char * initrd_getmod(char * modname);
extern char * initrd_getmod(int mod_id);
extern int initrd_modcount(void);

#endif /* SRC_INITRD_H_ */
