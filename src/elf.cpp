/*
 * elf.cpp
 *
 *  Created on: 22/03/2016
 *      Author: Miguel
 */

#include <elf.h>
#include <system.h>

#define BASE_OFF(header) ((uintptr_t)header)

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header) {
	return (elf32_shdr*) (BASE_OFF(elf_header) + elf_header->e_shoff);
}

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header, char section_idx) {
	return &(elf_section(elf_header)[section_idx]);
}

static inline elf32_shdr * elf_section(elf32_ehdr * elf_header, SH_TYPES section_type) {
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
	return elf_section(header, SHT_SYMTAB);
}

static inline elf32_sym * elf_symtable(elf32_ehdr * header) {
	elf32_shdr * symsection = elf_symsection(header);
	return symsection ? (elf32_sym*)(BASE_OFF(header) + symsection->sh_offset) : 0;
}

static inline elf32_sym * elf_sym(elf32_ehdr * header, int symidx) {
	return &elf_symtable(header)[symidx];
}

static inline int elf_section_entry_count(elf32_shdr * section) {
	return section->sh_size / section->sh_entsize;
}

static inline elf32_sym * elf_sym(elf32_ehdr * header, char * symname) {
	elf32_sym * sym = 0;
	for(int i = 0; i < elf_section_entry_count(elf_symsection(header)); i++)
		if(!strcmp(symname, elf_lookup_string(header,  (sym = elf_sym(header, i)))))
			return sym;
	return sym;
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, int index) {
	int symtab_entries = elf_section_entry_count(symsection);
	if(index >= symtab_entries) return 0;

	elf32_sym * symbol = elf_sym(header, index);
	return (BASE_OFF(header) + symbol->st_value + elf_section(header, symbol->st_shndx)->sh_offset);
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, char * symname, char charlimit, char calcoff, int greedy_ptr) {
	for(int i = 0; i < elf_section_entry_count(symsection);i++) {
		elf32_sym * symbol = elf_sym(header, i);
		char * symname_temp = elf_lookup_string(header, symbol);
		if(charlimit) {
			char * delim = strchr(symname_temp, (int)charlimit);
			if(delim) {
				int delimindex = (int)((uint32_t)delim - (uint32_t)symname_temp);
				if(!delimindex++) continue;

				char * symbuff = (char*)malloc(delimindex+1);
				memcpy(symbuff, symname_temp, delimindex);
				symbuff[delimindex] = '\0';

				if(!strcmp(symbuff, symname)) {
					if(greedy_ptr--) continue; /* Ignore this match */
					free(symbuff);
					if(calcoff)
						return (BASE_OFF(header) + symbol->st_value + elf_section(header, symbol->st_shndx)->sh_offset);
					else
						return symbol->st_value;
				}
				free(symbuff);
			}
		} else {
			if(!strcmp(symname, symname_temp)) {
				if(greedy_ptr--) continue; /* Ignore this match */
				if(calcoff)
					return (BASE_OFF(header) + symbol->st_value + elf_section(header, symbol->st_shndx)->sh_offset);
				else
					return symbol->st_value;
			}
		}
	}
	return 0;
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, char * symname, char charlimit) {
	return elf_get_symval(header, symsection, symname, charlimit, 1, 0);
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, char calcoff, char * symname) {
	return elf_get_symval(header, symsection, symname, 0, calcoff, 0);
}

static inline uintptr_t elf_get_symval(elf32_ehdr * header, elf32_shdr * symsection, char * symname) {
	return elf_get_symval(header, symsection, symname, 0);
}

modent_t * elf_find_mod(elf32_ehdr * header) {
	modent_t * mod = (modent_t*)elf_get_symval(header, elf_section(header, SHT_SYMTAB), STR(MODULE_SIGNATURE0), MODULE_SIGNATURE1);

	/* Check if it is a dependency: */
	if(!mod) {
		mod = (modent_t*)elf_get_symval(header, elf_section(header, SHT_SYMTAB), STR(MODULE_SIGNATUREXT0), MODULE_SIGNATURE1);
		return  (modent_t *)(mod ? MOD_DEP : MOD_UNKNOWN); /* If it's a dependency we don't want it... */
	}

	/* The module was found */

	/* Add desired symbols to the symbol table: */
	int sym_ctr = 0;
	sym_t * sym = (sym_t*)elf_get_symval(header, elf_symsection(header), "sym_", '_', 1, sym_ctr);
	while(sym) {
		symbol_add(sym->name, sym->addr);
		sym = (sym_t*)elf_get_symval(header, elf_symsection(header), "sym_", '_', 1, ++sym_ctr); /* Fetch the next symbol */
	}
	return mod;
}

/* Load ELF file that is an executable and was linked: */
char elf_load_exec(elf32_ehdr * elf_header) {
	if(!elf32_is_elf((uint8_t*)elf_header)) return 0;
	if(elf_header->e_type == ET_REL) return elf_relocate(elf_header);


	return 0;
}

#define DO_386_32(S, A)	((S) + (A))
#define DO_386_PC32(S, A, P) ((S) + (A) - (P))

/* Load ELF file that was compiled with the flag -c.
 * It wasn't linked and in order to make it 'runnable',
 * it must be relocated: */
char elf_relocate(elf32_ehdr * elf_header) {
	if(!elf32_is_elf((uint8_t*)elf_header)) return 0;
	if(elf_header->e_type == ET_EXEC) return elf_load_exec(elf_header);

	for(int i = 0; i < elf_header->e_shnum; i++) {
		elf32_shdr * rel_sect = elf_section(elf_header, i);
		if(rel_sect->sh_type == SHT_REL) {
			elf32_rel * reltab = (elf32_rel*)(BASE_OFF(elf_header) + rel_sect->sh_offset);
			for(int j = 0; j < elf_section_entry_count(rel_sect); j++) {
				elf32_shdr * target = elf_section(elf_header, rel_sect->sh_info);

				uintptr_t * ref = (uintptr_t*)((BASE_OFF(elf_header) + target->sh_offset) + reltab[j].r_offset);

				int symval = 0;
				if(ELF32_R_SYM(reltab[j].r_info) != SHN_UNDEF) {
					symval = elf_get_symval(elf_header, elf_section(elf_header, rel_sect->sh_link), ELF32_R_SYM(reltab[j].r_info));
					if(!symval) continue;
				}

				switch(ELF32_R_TYPE(reltab[j].r_info)) {
				case R_386_NONE: break;
				case R_386_32:
					/* Symbol + Offset: */
					*ref = DO_386_32(symval, *ref);
					break;
				case R_386_PC32:
					*ref = DO_386_PC32(symval, *ref, (int)ref);
					break;
				default: continue;
				}
			}
		}
	}
	return 1;
}

/* Load any ELF file. The function will
 * try to do its best: */
char elf_load(void * file) {
	if(!file) return 0;
	char retval;
	elf32_ehdr * hdr = (elf32_ehdr*)file;
	if((retval = elf32_is_supported(hdr))) return retval;

	switch(hdr->e_type) {
	case ET_EXEC: return elf_load_exec(hdr);
	case ET_REL : return elf_relocate(hdr);
	}
	return -1;
}

typedef int (*call_t)(int, char**);
call_t c = 0;

char * elf_parse(uint8_t * blob, int blobsize) {
	elf32_ehdr * hdr = (elf32_ehdr*)blob;

	for(uintptr_t i = 0; i < (uintptr_t)hdr->e_shentsize * hdr->e_shnum; i += hdr->e_shentsize) {
		elf32_shdr * shdr = (elf32_shdr*)((uintptr_t)hdr + (hdr->e_shoff + i));
		if(shdr->sh_addr) {
			for (uintptr_t i = 0; i < shdr->sh_size + 0x2000; i += 0x1000) {
				alloc_page(1, 1, shdr->sh_addr + i,shdr->sh_addr + i);
				invalidate_tables_at(shdr->sh_addr + i);
			}
			if(shdr->sh_type == SHT_NOBITS) {
				memset((void *)(shdr->sh_addr), 0x0, shdr->sh_size);
			} else {
				//kprintf("\nmemcpy to 0x%x from 0x%x size: %d\n", shdr->sh_addr, (uintptr_t)hdr + shdr->sh_offset, shdr->sh_size);
				memcpy((void*)shdr->sh_addr, (void*)((uintptr_t)hdr + shdr->sh_offset), shdr->sh_size);
			}
		}
	}


	c = (call_t)hdr->e_entry;
	kprintf("\nEntry: 0x%x\n", c);
	c(0,0);
	kprintf("RETURNED");
	return 0;
}
