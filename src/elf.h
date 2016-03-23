/*
 * elf.h
 *
 *  Created on: 22/03/2016
 *      Author: Miguel
 */

#ifndef SRC_ELF_H_
#define SRC_ELF_H_

#include <libc.h>

/* This file is defined as specified in the following document: http://www.skyfree.org/linux/references/ELF_Format.pdf */

#define EI_NIDENT 16
#define ELF_MAG 0x7F
#define ELF_SIGN "ELF"

namespace elf32_types {
	typedef uint32_t Elf32_addr; /* Program address */
	typedef uint16_t Elf32_half; /* Medium integer */
	typedef uint32_t Elf32_off; /* File offset */
	typedef int32_t  Elf32_sword; /* Large integer */
	typedef uint32_t Elf32_word; /* Large integer */
} using namespace elf32_types;

typedef struct {
	uint8_t    e_ident[EI_NIDENT];
	Elf32_half e_type;
	Elf32_half e_machine;
	Elf32_word e_version;
	Elf32_addr e_entry;
	Elf32_off  e_phoff;
	Elf32_off  e_shoff;
	Elf32_word e_flags;
	Elf32_half e_ehsize;
	Elf32_half e_phentsize;
	Elf32_half e_phnum;
	Elf32_half e_shentsize;
	Elf32_half e_shnum;
	Elf32_half e_shstrndx;
} elf32_header;

inline char elf32_is_elf(char * file_blob) {
	char signbuff[5];
	memcpy(signbuff, file_blob, 4);
	signbuff[4] = 0;
	return (!strcmp(signbuff + 1, ELF_SIGN) && signbuff[0] == ELF_MAG);
}

extern char * elf_parse(char * blob, int blobsize);

#endif /* SRC_ELF_H_ */
