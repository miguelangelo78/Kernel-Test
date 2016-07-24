/*
 * symbol_db.h
 *
 *  Created on: 23/07/2016
 *      Author: Miguel
 */

#ifndef SRC_MODULES_SYMBOL_DB_H_
#define SRC_MODULES_SYMBOL_DB_H_

/*****************/
/** DB Requests **/
/*****************/
enum SYMREQS { /* What data would you like to fetch? */
	SYMREQ_UNKNOWN,
	SYMREQ_ADDR,     /* Request Symbol Address */
	SYMREQ_SYMNAME,  /* Request Symbol Name */
	SYMREQ_PROGNAME, /* Request Program Name */
	SYMREQ_LOADADDR, /* Request Program's loaded address (if it was never loaded, it'll be zeroed) */
	SYMREQ_PROG      /* Request an entire program_t struct */
};

enum SYMREQ_TYPE { /* Is the request a get or a set statement? And how large will the data flow be? */
	SYMREQT_UNKNOWN,
	SYMREQT_SET,   /* Set Primitive type (uint32_t) */
	SYMREQT_GET,   /* Get Primitive type (uint32_t) */
	SYMREQT_SETS,  /* Set structure (could be less, equal or larger than 32 bits) */
	SYMREQT_GETS   /* Get structure (could be less, equal or larger than 32 bits) */
};

enum SYMOP_TYPE { /* With what data do you want the search to be made with? */
	SYMOP_UNKNOWN,
	SYMOP_ADDR,     /* Search is made by using an address */
	SYMOP_SYMNAME,  /* Search is made by using a symbol name */
	SYMOP_SYMN,     /* Search is made by using a symbol index RELATIVE to a certain program. It'll set/get an nth symbol */
	SYMOP_SYMN_ABS, /* Search is made by using a symbol index RELATIVE to the FIRST program. It'll set/get an nth symbol */
	SYMOP_PROGNAME, /* Search is made by using the name of a program */
	SYMOP_LOADADDR, /* Search is made by using a loaded address */
	SYMOP_PROG,     /* Search is made by using the pointer of a program_t struct */
	SYMOP_PROGN     /* Search is made by using a program index. It'll set/get an nth program */
};
/*****************/


/*******************/
/** Program Types **/
/*******************/
enum PROG_TYPES {
	PROGTYPE_KERNEL, /* The program is the actual Kernel */
	PROGTYPE_OBJ,    /* The program is a relocatable obj file that was not linked */
	PROGTYPE_EXE     /* The program is an executable file that was linked and has an entry point */
};
/*******************/

/***********************/
/* Struct Definitions: */
/***********************/
typedef struct symbol {
	char * name;
	uintptr_t address;
} symbol_t;

typedef struct program {
	char * name;
	char * name_abs;
	symbol_t * symtable;
	uint32_t symtable_size;
	uint32_t size;
	uintptr_t load_addr;
	char type;
	char * asmsrc;
	char * csrc;
} program_t;
/***********************/

void symbol_db_query(void); /* TODO */

#endif /* SRC_MODULES_SYMBOL_DB_H_ */
