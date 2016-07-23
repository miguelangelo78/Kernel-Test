/*
 * symbol.cpp
 *
 *  Created on: 23/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>

#define MAX_SYMBOLS_PER_TABLE 2048
#define PROGCOUNT 5

/***********************/
/* Struct definitions: */
/***********************/
typedef struct symbol {
	char * name;
	uintptr_t address;
} symbol_t;

typedef struct program {
	char * name;
	uint32_t size;
	uintptr_t load_addr;
	uint32_t symtable_size;
} program_t;
/***********************/

static symbol_t symtable[][PROGCOUNT] = {
/***************************************/
/*<GENBEGIN_PROG>*/
{
	{"sym1", 0xDEAF},
	{"sym2", 0xDEAF},
},
/*<GENEND_PROG>*/
/***************************************/
{0}
};

static program_t proglist[] = {
/***************************************/
/*<GENBEGIN_SYMTABLE>*/
{"myprog", 0, 0, 0},
/*<GENEND_SYMTABLE>*/
/***************************************/
{0}
};

uint32_t progcount = 0;

int symbol_db_init(void) {
	/* Populate the data that was not populated by the compiler: */
	for(int i = 0; proglist[i].name; i++) {
		program_t * prog = &proglist[i];

		/* Count the number of symbols on this program: */
		for(int j = 0; j < MAX_SYMBOLS_PER_TABLE; j++) {
			if(!symtable[i][j].name && !symtable[i][j].address) {
				prog->symtable_size = j;
				break;
			}
		}
		progcount++;
	}
	return 0;
}

static int symbol_db_finit(void) {

	return 0;
}

static uintptr_t symbol_db_ioctl(void * ioctl_packet) {

	return 0;
}

MODULE_DEF(symbol_db_driver, symbol_db_init, symbol_db_finit, MODT_DB, "Miguel S.", symbol_db_ioctl);
