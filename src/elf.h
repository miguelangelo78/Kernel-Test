/*
 * elf.h
 *
 *  Created on: 22/03/2016
 *      Author: Miguel
 */

#ifndef SRC_ELF_H_
#define SRC_ELF_H_

#define ELF_MAG 0x7F
#define ELF_SIGN "ELF"

typedef struct {
	char signature[4];

} elf32_header;

inline char elf32_is_elf(char * file_blob) {
	char signbuff[5];
	memcpy(signbuff, file_blob, 4);
	signbuff[4] = 0;
	return (!strcmp(signbuff + 1, ELF_SIGN) && signbuff[0] == ELF_MAG);
}

#endif /* SRC_ELF_H_ */
