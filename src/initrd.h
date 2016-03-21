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
	unsigned short header_size;
	unsigned short file_count;
	unsigned short offset, length;
	char filename;
} __packed initrd_header_t;

extern FILE * initrd_init(uint32_t location);
extern char * initrd_getmod(char * modname);
extern int initrd_modcount(void);

#endif /* SRC_INITRD_H_ */
