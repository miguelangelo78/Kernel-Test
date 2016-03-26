/*
 * module.cpp
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <elf.h>
#include <module.h>

namespace Module {

void modules_load(void) {
	int modcount = initrd_modcount();
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

			if(modentry && modentry != (modent_t *)MOD_DEP && modentry->init) { kprintf(" > ret: %d", modentry->init()); }
			else { continue; } /* We don't run modules that don't have an entry point */
		} else {
			kprintf(" | ERROR: Failed loading ELF");
		}
	}

	if(modcount) kprintf("\n\n >> ");
}

}
