/*
 * module.cpp
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <elf.h>

namespace Module {

void modules_load(void) {
	int modcount = initrd_modcount();
	kprintf("\n - Total modules: %d", modcount);

	for(int i = 0; i < modcount; i++) {
		FILE * mod = initrd_getmod_file(i);
		char * modblob = initrd_readfile(mod, 1);
		char is_elf = elf32_is_elf(modblob);

		kprintf("\n * %d - Module (%s): %s", i+1, mod->name, is_elf ? "VALID ELF" : "!INVALID ELF!");

		if(!is_elf) continue;

		/* Load up module and run its init function! */
		elf_parse(modblob, mod->size);
	}

	if(modcount) kprintf("\n\n");
}

}
