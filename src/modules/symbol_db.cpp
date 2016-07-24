/*
 * symbol.cpp
 *
 *  Created on: 23/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <libc/slre.h>
#include <module.h>
#include "symbol_db.h"

$LIBS(libc.o, libcc.o, string.o, slre.o)

#define MAX_SYMBOLS_PER_TABLE 2048

static uint32_t progcount;
static list_t * proglist;

/*******************************************/
/***** Symbol DB Module Init Functions *****/
/*******************************************/
static int symbol_db_init(void) {
	proglist = list_create();
	progcount = 0;
	return 0;
}

static int symbol_db_finit(void) {
	return 0;
}

static uintptr_t symbol_db_ioctl(void * ioctl_packet) {
	SWITCH_IOCTL(ioctl_packet) {
		case SYMREQ_ADDR: return 0;
	}
	return IOCTL_NULL;
}
/*******************************************/

MODULE_DEF(symbol_db_driver, symbol_db_init, symbol_db_finit, MODT_DB, "Miguel S.", symbol_db_ioctl);
