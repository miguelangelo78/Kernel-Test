/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

char hello[] = "hello world";

#define VID_CALC_POS(x, y) (x + y * VID_WIDTH)

char var[1000248]; /* Testing bss */

static int mod_ini(void) {
	var[0] = 1;
	char * vidmem = (char*)0xB8000;

	for(int i=0;i<10;i++) {
		for(int j=0;j<10;j++){
			int loc = VID_CALC_POS(i, j);
				vidmem[loc * 2] = 'c';
				vidmem[loc * 2 + 1] = 0;
		}
	}

	for(;;);

	return 0;
}

static int mod_fini(void) {
	return 0;
}

MODULE_DEF(modtest, mod_ini, mod_fini);
