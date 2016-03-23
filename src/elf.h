/*
 * elf.h
 *
 *  Created on: 22/03/2016
 *      Author: Miguel
 */

#ifndef SRC_ELF_H_
#define SRC_ELF_H_

#include <libc.h>

/*************************************************************************************************************************
 *                                                                                                                       *
 *  This file is defined as specified in the following document: http://www.skyfree.org/linux/references/ELF_Format.pdf  *
 *                                                                                                                       *
 *************************************************************************************************************************/

/* This header has been organized the following way: */
/* [ELF START HEADER] { Includes the specifications of the ELF object itself } */
/* [ELF PROGRAM HEADER] { Includes just the specifications for the headers/table of a Program }  */
/* [ELF SECTION HEADER] { Includes just the specifications for the headers/table of a Section } */
/* [ELF SYM TABLE] { Includes only information about the Symbol table } */
/* [ELF RELOCATION] { Includes only information about the relocation of symbols } */

/* Namespaces are being used solely to organize as much as possible each section of an ELF object file.
 * This is so it can be understood easily by the programmer and anyone reading the code.
 * If it is confusing please let me know. */

/* ELF 32 Data types: */
namespace elf32_types {
	typedef uint32_t Elf32_addr; /* Program address */
	typedef uint16_t Elf32_half; /* Medium integer */
	typedef uint32_t Elf32_off; /* File offset */
	typedef int32_t  Elf32_sword; /* Large integer */
	typedef uint32_t Elf32_word; /* Large integer */
} using namespace elf32_types;

/******************** ELF DEFINITION (BEGIN) ********************/

namespace ELF_START_HEADER { /* Encapsulate ALL the following definitions inside the actual ELF's header's namespace */
namespace ELF_MAGIC { /* Used for main identification of the object file */
	enum ELFCLASS {
		ELFCLASSNONE, /* Invalid class */
		ELFCLASS32, /* 32-Bit object */
		ELFCLASS64 /* 64-Bit object */
	};

	enum ELFENCODING {
		ELFDATANONE, /* Invalid data encoding */
		ELFDATA2LSB, /* LSB is on the lowest address */
		ELFDATA2MSB /* MSB is on the lowest address */
	};

	enum E_IDENTS {
		EI_MAG0, EI_MAG1, EI_MAG2, EI_MAG3, /* File ID */
		EI_CLASS, /* File class */
		EI_DATA, /* Data encoding */
		EI_VERSION, /* File Version */
		EI_PAD, /* Start of padding bytes */
		EI_NIDENT = 16 /* Size of e_ident[] */
	};

	enum MAGIC {
		ELFMAG0 = 0x7F, ELFMAG1 = 'E', ELFMAG2 = 'L', ELFMAG3 = 'F'
	};

	const char ELF_SIGN[4] = {ELFMAG1, ELFMAG2, ELFMAG3, 0};
} using namespace ELF_MAGIC;

enum E_TYPES {
	ET_NONE, /* No file type */
	ET_REL,  /* Relocatable file */
	ET_EXEC, /* Executable file */
	ET_DYN, /* Shared object file */
	ET_CORE, /* Core file */
	ET_LOPROC = 0xFF00, ET_HIPROC = 0xFFFF /* Processor specific */
};

enum E_MACHINES {
	EM_NONE, /* No machine */
	EM_M32, /* AT&T WE 32100 */
	EM_SPARC, /* SPARC */
	EM_386, /* Intel 80386 */
	EM_68K, /* Motorola 68000 */
	EM_88K, /* Motorola 88000 */
	EM_860, /* Intel 80860 */
	EM_MIPS, /* MIPS RS3000 */
	EM_XYZ /* Reserved for new machines */
};

enum E_VERSIONS {
	EV_NONE, /* Invalid Version */
	EV_CURRENT /* Current Version */
};

typedef struct {
	uint8_t    e_ident[EI_NIDENT]; /* Used for identifiying the file */
	Elf32_half e_type; /* Object file type */
	Elf32_half e_machine; /* Target architecture */
	Elf32_word e_version; /* Object file version */
	Elf32_addr e_entry; /* Start process's address. If no entry was set, then this will be 0 */
	Elf32_off  e_phoff; /* Program header table offset */
	Elf32_off  e_shoff; /* Section header table offset */
	Elf32_word e_flags; /* Holds processor specific flags */
	Elf32_half e_ehsize; /* ELF's header size */
	Elf32_half e_phentsize; /* Program header's table entry size */
	Elf32_half e_phnum; /* Number of entries on the Program header table */
	Elf32_half e_shentsize; /* Section header's table entry size */
	Elf32_half e_shnum; /* Number of entries on the Section header table */
	Elf32_half e_shstrndx; /* Entry's index on the Section header table */
} elf32_header;

} using namespace ELF_START_HEADER;

namespace ELF_PROGRAM_HEADER {

} using namespace ELF_PROGRAM_HEADER;

namespace ELF_SECTION_HEADER {

} using namespace ELF_SECTION_HEADER;

namespace ELF_SYM_TABLE {

} using namespace ELF_SYM_TABLE;

namespace ELF_SYM_REL {

} using namespace ELF_SYM_REL;

/******************** ELF DEFINITION (END) ********************/

inline char elf32_is_elf(char * file_blob) {
	char signbuff[5];
	memcpy(signbuff, file_blob, 4);
	signbuff[4] = 0;
	return (!strcmp(signbuff + 1, ELF_SIGN) && signbuff[0] == ELFMAG0);
}

extern char * elf_parse(char * blob, int blobsize);

#endif /* SRC_ELF_H_ */
