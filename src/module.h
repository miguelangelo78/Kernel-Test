#pragma once

#include <va_list.h>
#include <stdint.h>

/********** MODULES **********/
#define MODULE_SIGNATURE0 modent_
#define MODULE_SIGNATURE1 '_'
#define MODULE_SIGNATUREXT0 modentext_

#define MODULE_DEF(name, ini, fini) modent_t modent_ ## name = { STR(MODULE_SIGNATURE0) #name, ini, fini }
#define MODULE_EXT(name) modent_t modentext_ ## name = { STR(MODULE_SIGNATURE0) #name, 0, 0 }

/* This macro is only used by the genmake script */
#define MODULE_DEPS(...)

typedef int (*mod_init_t)(void);
typedef int (*mod_fini_t)(void);

typedef struct {
	char name[23];
	mod_init_t init;
	mod_fini_t finit;
} modent_t;

enum MOD_TYPE {
	MOD_UNKNOWN, MOD_CORE, MOD_DEP
};

/********** SYMBOLS **********/
#define KERNEL_SYMBOLS_TABLE_START 0x100000 /* Very important macro!! */
/* We reserved 2 pages for the symbol table at the very start of the kernel: */
#define KERNEL_SYMBOLS_TABLE_SIZE (0x1000 * 2) /* Each entry on the table is 8 bytes. This means the number of symbols that we can export is: TABLE_SIZE (in KiB) / 8 bytes  */
#define KERNEL_SYMBOLS_TABLE_END (KERNEL_SYMBOLS_TABLE_START + KERNEL_SYMBOLS_TABLE_SIZE) /* Very important macro!! */

#define EXPORT_SYMBOL(sym) \
	sym_t _sym_## sym __attribute__((section(".symbols"))) = {(char*)#sym, (uintptr_t)&sym}

/* Calculate the next/previous symbol's address: */
#define SYM(i) ((sym_t*)((unsigned int)KERNEL_SYMBOLS_TABLE_START + (sizeof(unsigned int) * 2) * i))
#define SYM_NEXT(sym_ptr) ((sym_t*)((unsigned int)sym_ptr + (sizeof(sym_ptr->name) + sizeof(sym_ptr->addr))))
#define SYM_NEXT_I(sym_ptr, i) ((sym_t*)((unsigned int)sym_ptr + (sizeof(sym_ptr->name) + sizeof(sym_ptr->addr)) * i))
#define SYM_PREV(sym_ptr) (((sym_t*)((unsigned int)sym_ptr - (sizeof(sym_ptr->name) + sizeof(sym_ptr->addr)))))
#define SYM_PREV_I(sym_ptr, i) ((sym_t*)((unsigned int)sym_ptr - (sizeof(sym_ptr->name) + sizeof(sym_ptr->addr)) * i))

/* Used to iterate the symbol section and to declare symbols: */
typedef struct {
	char * name;
	unsigned int addr;
} __attribute__((packed)) sym_t;

static inline void * symbol_find(char * name, char get_addr) {
	static sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
	int sym_ctr = 0;
	for(int i = 0; i < KERNEL_SYMBOLS_TABLE_SIZE / sizeof(sym_t); i++)
		if(!strcmp(sym[i].name, name))
			return get_addr ? (void*)sym[i].addr : &sym[i];
	/* Symbol not found: */
	return 0;
}

static inline void * symbol_find(char * name) {
	return symbol_find(name, 1);
}

static inline sym_t * symbol_t_find(char * name) {
	return (sym_t*)symbol_find(name, 0);
}

#define MAX_ARGUMENT 10

#define symbol_call_args(function_name, ...) symbol_call_args_(#function_name, PP_NARG(__VA_ARGS__), __VA_ARGS__)

#define SYA(function_name, ...) symbol_call_args(function_name, __VA_ARGS__)
#define SYC(function_nama) symbol_call(function_name)

static inline void * symbol_call_args_(char * function_name, int argc, ...) {
	if(argc <= 0 || argc >= MAX_ARGUMENT) return 0;

	void * ret = 0;
	int i;
	typedef void * (*cback)(void*);
	cback symbol = (cback)symbol_find(function_name);

	va_list args;
	va_start(args, function_name);

	/* Store arguments backwards: */
	unsigned int arglist[MAX_ARGUMENT];
	for(i = argc - 1;i >= 0; i--)
		arglist[i] = va_arg(args, unsigned int);

	/* With assembly: */
	for(i = 0; i < argc; i++) /* Push argument */
		asm volatile("mov %0,%%eax; push %%eax" : : "r"(arglist[i]) : "%eax");

	/* Call function */
	asm volatile("call *%0" : : "r"(symbol));

	/* Pop stack: */
	for(i = 0; i < argc; i++) asm volatile("pop %ebp");

	va_end(args);
	return 0;
}

static inline void * symbol_call(char * name) {
	typedef void * (*cback)(void);
	cback fptr = (cback)symbol_find(name);
	return fptr ? fptr() : 0;
}

static inline uint32_t symbol_count(void) {
	uint32_t symcount = 0;
	static sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
	while((unsigned int)sym < (unsigned int)KERNEL_SYMBOLS_TABLE_END) {
		if(sym->name) symcount++;
		sym = SYM_NEXT(sym);
	}
	return symcount;
}

/* Finds the next available symbol slot: */
static inline sym_t * symbol_next(void) {
static sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
	int sym_ctr = 0;
	for(int i = 0; i < KERNEL_SYMBOLS_TABLE_SIZE / sizeof(sym_t); i++)
		if(!sym[i].name)
			return &sym[i];
	/* Symbol not found: */
	return (sym_t*)0;
}

static inline char symbol_exists(char * name) {
	sym_t * sym = symbol_t_find(name);
	return sym ? (sym->name ? 1 : 0) : 0;
}

static inline char symbol_add(char * name, uintptr_t address) {
	if(symbol_exists(name)) return 0;
	sym_t * new_sym = symbol_next();
	if(!new_sym) return 0;
	new_sym->name = (char*)name;
	new_sym->addr = address;
	return 1;
}

static inline char symbol_remove(char * name) {
	if(!symbol_exists(name)) return 0;
	sym_t * sym_to_remove = symbol_t_find(name);
	if(!sym_to_remove) return 0;
	sym_to_remove->name = 0;
	sym_to_remove->addr = 0;
	return 1;
}

static inline sym_t * symbol_first() {
	sym_t * s = SYM(0);
	return s ? s : 0;
}

#ifdef __cplusplus
#ifndef MODULE

#include <system.h>
#include <stdint.h>

namespace Module {
	/********** MODULES **********/
	extern void modules_load(void);

	extern "C" { void kernel_symbols_start(void); }
	extern "C" { void kernel_symbols_end(void); }

	inline void * symbols_dump(void) {
		/* Only the kernel can call this function */
		sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
		kprintf("Symbol Section Start: 0x%x - End: 0x%x Size: 0x%x\n",
			KERNEL_SYMBOLS_TABLE_START,
			KERNEL_SYMBOLS_TABLE_END,
			(uintptr_t)KERNEL_SYMBOLS_TABLE_END - (uintptr_t)KERNEL_SYMBOLS_TABLE_START);
		
		for(int i = 1; (uintptr_t)sym < (uintptr_t)KERNEL_SYMBOLS_TABLE_END; i++) {
			kprintf("%d - %s: 0x%x\n", i, sym->name, sym->addr);
			sym = SYM_NEXT(sym);
		}
		kputs("Dump done\n");
		return 0;
	}
}

using namespace Module;

#endif
#endif
