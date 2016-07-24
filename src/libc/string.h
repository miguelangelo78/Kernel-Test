/*
 * string.h
 *
 *  Created on: 24/07/2016
 *      Author: Miguel
 */

#ifndef SRC_LIBC_STRING_H_
#define SRC_LIBC_STRING_H_

#include <stdint.h>

typedef struct {
	char ** str;
	int wordcount;
} split_t;

extern split_t split(char * str, char deli);
extern void free_split(split_t str);
extern char *trim(char *str);
extern char * str_replace(char *orig, char *rep, char *with);
extern uint8_t str_contains(char * str, char * needle);

#endif /* SRC_LIBC_STRING_H_ */
