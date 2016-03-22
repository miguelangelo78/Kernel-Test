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
	kprintf("\n - Total modules: %d\n", modcount);

	for(int i = 0; i < modcount; i++) {
		FILE * mod = initrd_getmod_file(i);
		char * modblob = initrd_readfile(mod, 1);

		kprintf(" * %d - Module (%s): %s", i+1, mod->name, elf32_is_elf(modblob) ? "VALID ELF" : "INVALID ELF");

		/* Load up module and run its init function! */


		kprintf("\n");
	}
}

}
