/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

#define VID_CALC_POS(x, y) (x + y * 80)
static char * vidmem = (char*)0xB8000;

int mod_ini(void) {
	int i,j;
	for(i=0;i<10;i++) {
		for(j=0;j<10;j++){
			int loc = VID_CALC_POS(i, j);

			vidmem[loc * 2] = 'c';
				vidmem[loc * 2 + 1] = 0;
		}
	}

	for(;;);

	return 0;
}

int mod_fini(void) {
	return 0;
}

MODULE_DEF(mod_test, mod_ini, mod_fini);
