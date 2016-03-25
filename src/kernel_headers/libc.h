/*
 * libc.h
 *
 *  Created on: 25/03/2016
 *      Author: Miguel
 */

#ifndef SRC_KERNEL_HEADERS_LIBC_H_
#define SRC_KERNEL_HEADERS_LIBC_H_

#define STRSTR(str) #str
#define STR(str) STRSTR(str)

inline int strcmp(const char * l, const char * r) {
	for (; *l == *r && *l; l++, r++);
	return *(unsigned char *)l - *(unsigned char *)r;
}

#endif /* SRC_KERNEL_HEADERS_LIBC_H_ */
