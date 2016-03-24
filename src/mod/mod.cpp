/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

char hello[] = "hello world";
char find[] = "findme";

#define VID_CALC_POS(x, y) (x + y * VID_WIDTH)

static int mod_ini(void) {
	for(;;);

	return 0;
}

static int mod_fini(void) {
	return 0;
}

MODULE_DEF(modtest, mod_ini, mod_fini);
