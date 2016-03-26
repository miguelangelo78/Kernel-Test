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
		elf_relocate((elf32_ehdr*)modblob);
		/* Load up module and run its init function! */
		modent_t * modentry = (modent_t *)elf_find_mod((elf32_ehdr*)modblob);
		kprintf(" | Initializing...", strchr(modentry->name, MODULE_SIGNATURE1)+1);
		modentry->init();
	}

	if(modcount) kprintf("\n\n >> ");
}

}
