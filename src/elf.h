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
} elf32_ehdr;

} using namespace ELF_START_HEADER;

namespace ELF_PROGRAM_HEADER {
	enum P_TYPES {
		PT_NULL, /* The array element is unused; other members’ values are undefined. This type lets the program header table have ignored entries*/
		PT_LOAD, /* The array element specifies a loadable segment, described by p_filesz and p_memsz. */
		PT_DYNAMIC, /* The array element specifies dynamic linking information. */
		PT_INTERP, /* The array element specifies the location and size of a null-terminated path name to invoke as an interpreter */
		PT_NOTE, /* The array element specifies the location and size of auxiliary information. */
		PT_SHLIB, /* This segment type is reserved but has unspecified semantics. Programs that contain an array element of this type do not conform to the ABI */
		PT_PHDR, /* The array element, if present, specifies the location and size of the program header table itself, both in the file and in the memory image of the program */
		PT_LOPROC = 0x70000000, PT_HIPROC = 0x7fffffff /* Values in this inclusive range are reserved for processor-specific semantics. */
	};

	typedef struct {
		Elf32_word p_type; /* This member tells what kind of segment this array element describes or how to interpret the array element’s information */
		Elf32_off  p_offset; /* This member gives the offset from the beginning of the file at which the first byte of the segment resides */
		Elf32_addr p_vaddr; /* This member gives the virtual address at which the first byte of the segment resides in memory */
		Elf32_addr p_paddr; /* On systems for which physical addressing is relevant, this member is reserved for the segment’s physical address */
		Elf32_word p_filesz; /* This member gives the number of bytes in the file image of the segment; it may be zero. */
		Elf32_word p_memsz; /* This member gives the number of bytes in the memory image of the segment; it may be zero. */
		Elf32_word p_flags; /* This member gives flags relevant to the segment. */
		Elf32_word p_align; /* This member gives the value to which the segments are aligned in memory and in the file. */
	} elf32_phdr;
} using namespace ELF_PROGRAM_HEADER;

namespace ELF_SECTION_HEADER {
	enum SHN_SECTION_INDEX {
		SHN_UNDEF, /* Missing reference */
		SHN_LORESERVE = 0xFF00, /* Lower bound of the range of reserved indexes */
		SHN_LOPROC = 0xFF00,
		SHN_HIPROC = 0xFF1F,
		SHN_ABS = 0xFFF1, /* Absolute values for the references */
		SHN_COMMON = 0xFFF2, /* Common symbols */
		SHN_HIRESERVE = 0xFFFF  /* Upper bound of the range of reserved indexes */
	};

	enum SH_TYPES {
		SHT_NULL, /* The section is inactive */
		SHT_PROGBITS, /* The section holds info defined by the program */
		SHT_SYMTAB, /* The section holds a symbol table */
		SHT_STRTAB, /* The section holds a string table */
		SHT_RELA,/* The section holds relocation entries */
		SHT_HASH, /* The section holds a symbol hash table */
		SHT_DYNAMIC, /* This section holds information for dynamic linking */
		SHT_NOTE, /* This section holds information that marks the file in some way. */
		SHT_NOBITS, /* A section of this type occupies no space in the file but otherwise resembles SHT_PROGBITS. */
		SHT_REL, /* The section holds relocation entries without explicit addends, such as type Elf32_Rel for the 32-bit class of object files. */
		SHT_SHLIB, /* This section type is reserved but has unspecified semantics */
		SHT_DYNSYM, /* The section holds a symbol table */
		SHT_LOPROC = 0x70000000, /* Values in this inclusive range are reserved for processor-specific semantics */
		SHT_HIPROC = 0x7fffffff, /* Values in this inclusive range are reserved for processor-specific semantics */
		SHT_LOUSER = 0x80000000, /* This value specifies the lower bound of the range of indexes reserved for application programs. */
		SHT_HIUSER = 0xffffffff  /* This value specifies the upper bound of the range of indexes reserved for application programs */
	};

	enum ELF_SECTIONS {
		_BSS = SHT_NOBITS, /* Uninitialized data [SHF_ALLOC + SHF_WRITE] */
		_COMMENT = SHT_PROGBITS, /* Version Control Information */
		_DATA = SHT_PROGBITS, /* Initialized data [SHF_ALLOC + SHF_WRITE] */
		_DATA1 = SHT_PROGBITS, /* Initialized data [SHF_ALLOC + SHF_WRITE] */
		_DEBUG = SHT_PROGBITS, /* Symbols for debugging */
		_DYNAMIC = SHT_DYNAMIC, /* Dynamic linking info */
		_DYNSTR = SHT_STRTAB, /* Strings needed for dynamic linking */
		_DYNSYM = SHT_DYNSYM, /* Dynamic symbol table */
		_FINI = SHT_PROGBITS, /* Instructions that contribut to the process termination */
		_GOT = SHT_PROGBITS, /* Global offset Table */
		_HASH = SHT_HASH, /* Holds a Symbol hash Table */
		_INIT = SHT_PROGBITS, /* Instructions that contribut to the process initialization */
		_INTERP = SHT_PROGBITS, /* This section holds the path name of a program interpreter */
		_LINE = SHT_PROGBITS, /* This section holds line number information for symbolic debugging, which describes the correspondence between the source program and the machine code */
		_NOTE = SHT_NOTE,
		_PLT = SHT_PROGBITS, /* This section holds the procedure linkage table.  */
		_REL_NAME = SHT_REL, /* These sections hold relocation information */
		_RELA_NAME = SHT_RELA, /* These sections hold relocation information */
		_RODATA = SHT_PROGBITS,  /* These sections hold read-only data that typically contribute to a non-writable segment in the process image. */
		_RODATA1 = SHT_PROGBITS, /* These sections hold read-only data that typically contribute to a non-writable segment in the process image. */
		_SHSTRTAB = SHT_STRTAB, /* This section holds section names. */
		_STRTAB = SHT_STRTAB, /* This section holds strings, most commonly the strings that represent the names associated with symbol table entries. */
		_SYMTAB = SHT_SYMTAB, /* This section holds a symbol table */
		_TEXT = SHT_PROGBITS /* This section holds the "text," or executable instructions, of a program */
	};

	enum SH_FLAGS {
		SHF_WRITE = 0x1, /* The section contains data that should be writable during process execution */
		SHF_ALLOC = 0x2, /* The section occupies memory during process execution */
		SHF_EXECINSTR = 0x4, /* The section contains executable machine instructions. */
		SHF_MASKPROC = 0xf0000000 /* All bits included in this mask are reserved for processor-specific semantics. */
	};

	typedef struct {
		Elf32_word sh_name; /* Section name */
		Elf32_word sh_type; /* Section type */
		Elf32_word sh_flags; /* Misc attributes */
		Elf32_addr sh_addr; /* Start of the Section's entry */
		Elf32_off  sh_offset; /* "This member’s value gives the byte offset from the beginning of the file to the first byte in the section." */
		Elf32_word sh_size; /* Section's size */
		Elf32_word sh_link; /* "This member holds a section header table index link" */
		Elf32_word sh_info; /* Extra info */
		Elf32_word sh_addralign; /* "Some sections have address alignment constraints. ... Values 0 and 1 mean the section has no alignment constraints" */
		Elf32_word sh_entsize; /* Section's entry size */
	} elf32_shdr;
} using namespace ELF_SECTION_HEADER;

namespace ELF_SYM_TABLE {
	#define SHN_UNDEF 0x00
	#define ELF32_ST_BIND(i) ((i) >> 4)
	#define ELF32_ST_TYPE(i) ((i) & 0xF)
	#define ELF32_ST_INFO(b,t)  (((b) << 4) + ((t) & 0xF)

	enum SYM_BIND {
		STB_LOCAL, /* Local symbols are not visible outside the object file containing their definition.  */
		STB_GLOBAL, /* Global symbols are visible to all object files being combined. */
		STB_WEAK, /* Weak symbols resemble global symbols, but their definitions have lower precedence. */
		STB_LOPROC = 13, STB_HIPROC = 15 /* Values in this inclusive range are reserved for processor-specific semantics */
	};

	enum SYM_TYPE {
		STT_NOTYPE, /* Symbol's type is not specified */
		STT_OBJECT, /* The symbol is associated with a data object, such as a variable, an array, etc */
		STT_FUNC, /* The symbol is associated with a function or other executable code. */
		STT_SECTION, /* The symbol is associated with a section. Symbol table entries of this type exist primarily for relocation and normally have STB_LOCAL binding */
		STT_FILE, /* Conventionally, the symbol's name gives the name of the source file associated with the object file.  */
		STT_LOPROC = 13, STT_HIPROC = 15 /* Values in this inclusive range are reserved for processor-specific semantics */
	};

	typedef struct {
		Elf32_word st_name;  /* Symbol's name */
		Elf32_addr st_value; /* Address of the symbol */
		Elf32_word st_size;  /* Symbol's size */
		uint8_t    st_info;  /* Symbol's type */
		uint8_t    st_other; /* Ignore this */
		Elf32_half st_shndx; /* Section's index */
	} elf32_sym;
} using namespace ELF_SYM_TABLE;

namespace ELF_SYM_REL {
	#define ELF32_R_SYM(i) ((i) >> 8)
	#define ELF32_R_TYPE(i) ((uint8_t)(i))
	#define ELF32_R_INFO(s,t) (((s) << 8) + (uint8_t)(t))

	enum REL_TYPES {
		R_386_NONE,
		R_386_32,
		R_386_PC32,
		R_386_GOT32, /* This relocation type computes the distance from the base of the global offset table to the symbol’s global offset table entry. */
		R_386_PLT32, /* This relocation type computes the address of the symbol’s procedure linkage table entry and additionally instructs the link editor to build a procedure linkage table. */
		R_386_COPY, /* The link editor creates this relocation type for dynamic linking. */
		R_386_GLOB_DAT, /* This relocation type is used to set a global offset table entry to the address of the specified symbol. */
		R_386_JMP_SLOT, /* The link editor creates this relocation type for dynamic linking. */
		R_386_RELATIVE, /* The link editor creates this relocation type for dynamic linking. */
		R_386_GOTOFF, /* This relocation type computes the difference between a symbol’s value and the address of the global offset table */
		R_386_GOTPC /* This relocation type resembles R_386_PC32, except it uses the address of the global offset table in its calculation.  */
	};

	typedef struct {
		Elf32_addr r_offset; /* This member gives the location at which to apply the relocation action */
		Elf32_word r_info;
	} elf32_rel;

	typedef struct {
		Elf32_addr r_offset; /* This member gives the location at which to apply the relocation action */
		Elf32_word r_info; /* This member gives both the symbol table index with respect to which the relocation must be made, and the type of relocation to apply */
		Elf32_sword r_addend; /* This member specifies a constant addend used to compute the value to be stored into the relocatable field. */
	} elf32_rela;
} using namespace ELF_SYM_REL;

/******************** ELF DEFINITION (END) ********************/

inline char elf32_is_elf(uint8_t * file_blob) {
	char signbuff[5];
	memcpy(signbuff, file_blob, 4);
	signbuff[4] = 0;
	return (!strcmp(signbuff + 1, ELF_SIGN) && signbuff[0] == ELFMAG0);
}

extern char * elf_parse(uint8_t * blob, int blobsize);

#endif /* SRC_ELF_H_ */
