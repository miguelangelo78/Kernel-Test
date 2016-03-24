/*
 * elf.cpp
 *
 *  Created on: 22/03/2016
 *      Author: Miguel
 */

#include <elf.h>
#include <system.h>
#include <module.h>

#define BASE_OFF(header) ((uintptr_t)header)

static inline elf32_shdr * elf_sheader(elf32_ehdr * elf_header) {
	return (elf32_shdr*) (BASE_OFF(elf_header) + elf_header->e_shoff);
}

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header, char section_idx) {
	return &(elf_sheader(elf_header)[section_idx]);
}

inline char * elf_str_table(elf32_ehdr * elf_header) {
	return elf_header->e_shstrndx == SHN_UNDEF ? 0 : (char*)BASE_OFF(elf_header) + elf_section(elf_header, elf_header->e_shstrndx)->sh_offset;
}

static inline char * elf_lookup_string(elf32_ehdr *elf_header, elf32_shdr * elf_section) {
	char *strtab = elf_str_table(elf_header);
	return strtab ? strtab + elf_section->sh_name : 0;
}

static inline char * elf_lookup_string(elf32_ehdr *elf_header, elf32_sym * elf_symtable) {
	char *strtab = elf_str_table(elf_header);
	return strtab ? strtab + elf_symtable->st_name : 0;
}

static inline elf32_shdr * elf_symsection(elf32_ehdr * header) {
	elf32_shdr * symsection;
	for(int i = 0;i < header->e_shnum; i++)
		if((symsection = elf_section(header, i))->sh_type == SHT_SYMTAB) return symsection;
	return 0;
}

static inline elf32_sym * elf_symtable(elf32_ehdr * header) {
	elf32_shdr * symsection = elf_symsection(header);
	return symsection ? (elf32_sym*)((uint32_t)header + symsection->sh_offset) : 0;
}

static inline elf32_sym * elf_sym(elf32_ehdr * header, int symidx) {
	return &elf_symtable(header)[symidx];
}

static inline int elf_symtab_entry_count(elf32_shdr * symsection) {
	return symsection->sh_size / symsection->sh_entsize;
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, int index) {
	int symtab_entries = elf_symtab_entry_count(symsection);
	if(index >= symtab_entries) return 0;

	elf32_sym * symbol = elf_sym(header, index);
	return (BASE_OFF(header) + symbol->st_value + elf_section(header, symbol->st_shndx)->sh_offset);
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, char * symname, char charlimit) {
	for(int i = 0; i < elf_symtab_entry_count(symsection);i++) {
		elf32_sym * symbol = elf_sym(header, i);
		char * symname_temp = elf_lookup_string(header, symbol);
		if(charlimit) {
			char * delim = strchr(symname_temp, (int)charlimit);
			if(delim) {
				int delimindex = (int)((uint32_t)delim - (uint32_t)symname_temp);
				if(!delimindex++) continue;
				char * symbuff = (char*)malloc(delimindex);
				memcpy(symbuff, symname_temp, delimindex);
				symbuff[delimindex] = '\0';

				if(!strcmp(symbuff, symname)) {
					free(symbuff);
					return (BASE_OFF(header) + symbol->st_value + elf_section(header, symbol->st_shndx)->sh_offset);
				}
				free(symbuff);
			}
		} else {
			if(!strcmp(symname, symname_temp))
				return (BASE_OFF(header) + symbol->st_value + elf_section(header, symbol->st_shndx)->sh_offset);
		}
	}
	return 0;
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, char * symname) {
	return elf_get_symval(header, symsection, symname, 0);
}

char * elf_parse(uint8_t * blob, int blobsize) {
	elf32_ehdr * head = (elf32_ehdr*)blob;
	elf32_shdr * symsection = elf_symsection(head);
	elf32_sym * symtable = elf_symtable(head);

	kprintf("|%s|", elf_get_symval(head, symsection, "modent_", '_'));

	kprintf("\nDone");

	for(;;);
	return 0;
}
