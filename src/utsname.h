/*
 * utsname.h
 *
 *  Created on: 22/07/2016
 *      Author: Miguel
 */

#ifndef SRC_UTSNAME_H_
#define SRC_UTSNAME_H_

#define _UTSNAME_LENGTH 256

typedef struct utsname {
	char sysname   [_UTSNAME_LENGTH];
	char nodename  [_UTSNAME_LENGTH];
	char release   [_UTSNAME_LENGTH];
	char version   [_UTSNAME_LENGTH];
	char machine   [_UTSNAME_LENGTH];
	char domainname[_UTSNAME_LENGTH];
} utsname_t;

#endif /* SRC_UTSNAME_H_ */
