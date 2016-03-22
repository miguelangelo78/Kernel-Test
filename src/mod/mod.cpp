/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

int mod_ini(void) {
	kprintf("hello world");
	return 0;
}

int mod_fini(void) {
	return 0;
}

MODULE_DEF(modtest, mod_ini, mod_fini);
