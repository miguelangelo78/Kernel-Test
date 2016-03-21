/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <module.h>

static int mod_ini(void) {

	return 0;
}

static int mod_fini(void) {

}

MODULE_DEF(modtest, mod_ini, mod_fini);
