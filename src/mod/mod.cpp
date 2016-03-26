/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <kernel_headers/kheaders.h>
#include <module.h>

static int mod_ini(void) {
	kprintf("\n\ntest: %s %d %d\n", "test2", 2, 1);
	return 0;
}

static int mod_fini(void) {
	return 0;
}

MODULE_DEF(mod_test, mod_ini, mod_fini);

