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

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header) {
	return (elf32_shdr*) (BASE_OFF(elf_header) + elf_header->e_shoff);
}

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header, char section_idx) {
	return &(elf_section(elf_header)[section_idx]);
}

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header, SH_TYPES section_type, char ignore) {
	for(int i = 0;i < elf_header->e_shnum; i++) {
		elf32_shdr * sect = elf_section(elf_header, i);
		if(sect->sh_type == section_type) return sect;
	}
	return (elf32_shdr *)SHT_NULL;
}

/* Prototype: */
static inline char * elf_lookup_string(elf32_ehdr *elf_header, elf32_shdr * elf_sect);

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header, char * section_name) {
	for(int i = 0;i < elf_header->e_shnum; i++) {
		elf32_shdr * section = elf_section(elf_header, i);
		if(!strcmp(elf_lookup_string(elf_header, section), section_name)) return section;
	}
	return SHN_UNDEF;
}

static inline char * elf_string_table(elf32_ehdr * elf_header) {
	return (char*)(elf_header->e_shstrndx == SHN_UNDEF ? 0 : (BASE_OFF(elf_header) + elf_section(elf_header, elf_header->e_shstrndx)->sh_offset));
}

static inline char * elf_lookup_string(elf32_ehdr *elf_header, elf32_shdr * elf_sect) {
	char * strtab = elf_string_table(elf_header);
	return strtab ? strtab + elf_sect->sh_name: 0;
}

static inline char * elf_lookup_string(elf32_ehdr *elf_header, elf32_sym * elf_symtable) {
	char * strtab = (char*)(elf_header->e_shstrndx == SHN_UNDEF ? 0 : (BASE_OFF(elf_header) + elf_section(elf_header, ".strtab")->sh_offset));
	return strtab ? strtab + elf_symtable->st_name: 0;
}

static inline elf32_shdr * elf_symsection(elf32_ehdr * header) {
	return elf_section(header, SHT_SYMTAB, 0);
}

static inline elf32_sym * elf_symtable(elf32_ehdr * header) {
	elf32_shdr * symsection = elf_symsection(header);
	return symsection ? (elf32_sym*)(BASE_OFF(header) + symsection->sh_offset) : 0;
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

static inline modent_t * elf_find_mod(elf32_ehdr * header) {
	modent_t * mod = (modent_t*)elf_get_symval(header, elf_section(header, SHT_SYMTAB, 0), STR(MODULE_SIGNATURE0), MODULE_SIGNATURE1);
	if(!mod) return 0;

	/* Translate relative addresses to absolutes: */
	elf32_shdr * text_sect = elf_section(header, ".text");
	mod->init = (mod_init_t)(BASE_OFF(header)+(uintptr_t)mod->init + text_sect->sh_offset);
	mod->finit = (mod_init_t)(BASE_OFF(header)+(uintptr_t)mod->finit + text_sect->sh_offset);
	return mod;
}

char * elf_parse(uint8_t * blob, int blobsize) {
	elf32_ehdr * head = (elf32_ehdr*)blob;
	modent_t * mod = elf_find_mod(head);

	kprintf(" | Module name: %s\nRunning ... ret: 0x%x", strchr(mod->name, MODULE_SIGNATURE1)+1, mod->init());

	kprintf("\nDone");

	return 0;
}
