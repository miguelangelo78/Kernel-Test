/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

static int mod_ini(void) {

	static Terminal * t = (Terminal*)symbol_find("term");
	//t->puts("HELLO");

	return 2;
}

static int mod_fini(void) {
	return 0;
}

MODULE_DEF(mod_test, mod_ini, mod_fini);
