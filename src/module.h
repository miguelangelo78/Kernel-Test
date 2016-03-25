#pragma once

#define STRSTR(str) #str
#define STR(str) STRSTR(str)

#define MODULE_SIGNATURE0 modent_
#define MODULE_SIGNATURE1 '_'

#define MODULE_DEF(name, ini, fini) modent_t modent_ ## name = { STR(MODULE_SIGNATURE0) #name, &ini, &fini }

typedef int (*mod_init_t)(void);
typedef int (*mod_fini_t)(void);

typedef struct {
	char name[23];
	mod_init_t init;
	mod_fini_t finit;
} modent_t;

#ifdef __cplusplus

#include <system.h>
#include <stdint.h>

namespace Module {
	/********** MODULES **********/
	extern void modules_load(void);

	/********** SYMBOLS **********/
	#define KERNEL_SYMBOLS_TABLE_START 0x100000 /* Very important macro!! */
	/* We reserved 2 pages for the symbol table at the very start of the kernel: */
	#define KERNEL_SYMBOLS_TABLE_SIZE (PAGE_SIZE * 2) /* Each entry on the table is 8 bytes. This means the number of symbols that we can export is: TABLE_SIZE (in KiB) / 8 bytes  */
	#define KERNEL_SYMBOLS_TABLE_END (KERNEL_SYMBOLS_TABLE_START + KERNEL_SYMBOLS_TABLE_SIZE) /* Very important macro!! */
	extern "C" { void kernel_symbols_start(void); }
	extern "C" { void kernel_symbols_end(void); }

	#define EXPORT_SYMBOL(sym) \
		Module::sym_t _sym_## sym __attribute__((section(".symbols"))) = {(char*)#sym, (uintptr_t)&sym}

	/* Calculate the next symbol's address: */
	#define SYM_NEXT(sym_ptr) (sym_t*)((uintptr_t)sym_ptr + sizeof(sym_ptr->name) + sizeof(sym_ptr->addr))

	/* Used to iterate the symbol section and to declare symbols: */
	typedef struct {
		char * name;
		uintptr_t addr;
	} __packed sym_t;

	inline void * symbol_find(const char * name) {
		sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
		while((uintptr_t)sym < (uintptr_t)KERNEL_SYMBOLS_TABLE_END) {
			if (strcmp(sym->name, name)) {
				/* Fetch next symbol: */
				sym = SYM_NEXT(sym);
				continue;
			}
			return (void*)sym->addr;
		}
		return NULL;
	}

	inline void * symbols_dump(void) {
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

	inline void * symbol_call(const char * name, void * params) {
		typedef void * (*cback)(void*);
		cback fptr = (cback)Module::symbol_find(name);
		return fptr ? fptr(params) : NULL;
	}

	inline void * symbol_call(const char * name) {
		typedef void * (*cback)(void);
		cback fptr = (cback)Module::symbol_find(name);
		return fptr ? fptr() : NULL;
	}
}

using namespace Module;

#endif
