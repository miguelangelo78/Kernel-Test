/*
 * mod.cpp
 *
 *  Created on: 26/03/2016
 *      Author: Miguel
 */

#include <kernel_headers/kheaders.h>

// $DEPS(modules\mod2.mod)

extern char mydep;

static int ini(void) {
	kprintf("\nRUNNED >>%c<<\n", mydep);
	return 0;
}

static int fini(void) {
	return 0;
}

MODULE_DEF(mod, ini, fini);
