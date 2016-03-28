/*
 * module.cpp
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <elf.h>
#include <module.h>
#include <libc/list.h>

namespace Module {

list_t * modlist;
int modcount = 0;

#define GET_LMOD(n) ((modent_t*)(n->value))
#define MOD_IDX_INVALID(idx) ((idx) < 0 || (idx) > list_size(modlist))

uintptr_t module_ioctl(char * modname, void * data) {
	modent_t * mod = module_get(modname);
	return mod && mod->ioctl ? mod->ioctl(data) : (uintptr_t)-1;
}

uintptr_t module_ioctl_s(char * modname, void * data) {
	return module_ioctl(modname, data);
}
EXPORT_SYMBOL(module_ioctl_s);

uintptr_t module_ioctl(int mod_idx, void * data) {
	if(MOD_IDX_INVALID(mod_idx)) return 0;
	modent_t * mod = module_get(mod_idx);
	return mod && mod->ioctl ? mod->ioctl(data) : (uintptr_t)-1;
}

uintptr_t module_ioctl_i(int mod_idx, void * data) {
	return module_ioctl(mod_idx, data);
}
EXPORT_SYMBOL(module_ioctl_i);

modent_t * module_get(char * modname) {
	foreach(n, modlist)
		if(!strcmp(GET_LMOD(n)->name, modname)) return (modent_t*)n->value;
	return 0;
}

modent_t * module_gets(char * modname) {
	return module_get(modname);
}
EXPORT_SYMBOL(module_gets);

modent_t * module_get(int mod_idx) {
	if(MOD_IDX_INVALID(mod_idx)) return 0;
	return (modent_t*)list_get(modlist, mod_idx)->value;
}

modent_t * module_geti(int mod_idx){
	return module_get(mod_idx);
}
EXPORT_SYMBOL(module_geti);

int module_count(void) {
	return modcount;
}
EXPORT_SYMBOL(module_count);

char module_type_exists(char mod_type) {
	foreach(n, modlist)
		if(GET_LMOD(n)->type == mod_type) return 1;
	return 0;
}
EXPORT_SYMBOL(module_type_exists);

char module_exists(char * modname) {
	foreach(n, modlist)
		if(!strcmp(GET_LMOD(n)->name, modname)) return 1;
	return 0;
}

char module_exists_s(char * modname) {
	return module_exists(modname);
}
EXPORT_SYMBOL(module_exists_s);

char module_exists(modent_t * mod) {
	return list_find(modlist, mod) ? 1 : 0;
}

char module_add(modent_t * mod) {
	if(module_exists(mod)) return 0;
	list_insert(modlist, mod);
	modcount++;
	return 1;
}

char module_remove(char * modname) {
	int i = 0;
	foreach(n, modlist) {
		if(!strcmp(GET_LMOD(n)->name, modname)) {
			list_remove(modlist, i);
			modcount--;
			return 1;
		}
		i++;
	}
	return 0;
}

char module_remove(int mod_idx) {
	if(MOD_IDX_INVALID(mod_idx)) return 0;
	list_remove(modlist, mod_idx);
	modcount--;
	return 1;
}

char module_remove(modent_t * mod) {
	node_t * n = list_find(modlist, mod);
	if(n) {
		list_delete(modlist, n);
		modcount--;
		return 1;
	} else {
		return 0;
	}
}

void modules_load(void) {
	int modcount = initrd_modcount();
	modlist = list_create();

	kprintf("\n - Total modules: %d", modcount);

	for(int i = 0; i < modcount; i++) {
		FILE * mod = initrd_getmod_file(i);
		uint8_t * modblob = (uint8_t*)initrd_readfile(mod, 1);
		char is_elf = elf32_is_elf(modblob);

		kprintf("\n * %d - Module (%s): %s", i+1, mod->name, is_elf ? "VALID ELF" : "!INVALID ELF!");
		if(!is_elf) continue;

		/* Prepare elf file first: */
		if(elf_relocate((elf32_ehdr*)modblob)) {
			/* Load up module and run its init function! */
			modent_t * modentry = (modent_t *)elf_find_mod((elf32_ehdr*)modblob);
			if(modentry == (modent_t*)MOD_UNKNOWN) {
				kprintf(" | (UNKNOWN module)");
			}
			else {
				kprintf("%s", !modentry->init || modentry == (modent_t *)MOD_DEP ? " | (Dependable object)":" | Initializing...", strchr(modentry->name, MODULE_SIGNATURE1)+1);
			}

			if(modentry && modentry != (modent_t *)MOD_DEP && modentry->init) {
				kprintf(" > ret: %d", modentry->init());
				module_add(modentry);
			}
			else {
				continue;
			} /* We don't run modules that don't have an entry point */
		} else {
			kprintf(" | ERROR: Failed loading ELF");
		}
	}

	if(modcount) { kprintf("\n\n >> "); }
}

}
