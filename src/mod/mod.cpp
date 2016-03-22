/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

char hello[] = "hello world";

int mod_ini(void) {
	symbol_find("test");
	hello[0] = 'c';
	return 0;
}

int mod_fini(void) {
	return 0;
}

MODULE_DEF(modtest, mod_ini, mod_fini);
