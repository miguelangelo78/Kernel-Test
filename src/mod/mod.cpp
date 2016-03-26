/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <kernel_headers/kheaders.h>
#include <module.h>

static int mod_ini(void) {
	symbol_call_args("bridge", 3, "print me", "again!", 1);
	return 0;
}

static int mod_fini(void) {
	return 0;
}

MODULE_DEF(mod_test, mod_ini, mod_fini);

