#pragma once

#include <system.h>
#include <stdint.h>

namespace Module {
	extern "C" { void kernel_symbols_start(void); }
	extern "C" { void kernel_symbols_end(void); }

	#define EXPORT_SYMBOL(sym) \
		Module::sym_t _sym_## sym __attribute__((section(".symbols"))) = {(char*)#sym, (uintptr_t)&sym}

	/* Calculate the next symbol's address: */
	#define SYM_NEXT(sym_ptr) (sym_t*)((uintptr_t)sym_ptr + sizeof(sym_ptr->name) + sizeof(sym_ptr->addr))

	/* Used to iterate the symbol section and to declare symbols */
	typedef struct {
		char * name;
		uintptr_t addr;
	} __packed sym_t;

	inline void * symbol_find(const char * name) {
		sym_t * sym = (sym_t *)&kernel_symbols_start;
		while((uintptr_t)sym < (uintptr_t)&kernel_symbols_end) {
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
		sym_t * sym = (sym_t *)&kernel_symbols_start;
		Kernel::term.printf("Symbol Section Start: 0x%x - End: 0x%x Size: 0x%x\n", 
			&kernel_symbols_start, 
			&kernel_symbols_end, 
			(uintptr_t)&kernel_symbols_end - (uintptr_t)&kernel_symbols_start);
		
		for(int i = 1; (uintptr_t)sym < (uintptr_t)&kernel_symbols_end; i++) {
			Kernel::term.printf("%d - %s: 0x%x\n", i, sym->name, sym->addr);
			sym = SYM_NEXT(sym);
		}
		Kernel::term.puts("Dump done\n");
		return 0;
	}

	inline void * symbol_call(const char * name, void * params) {
		typedef void * (*cback)(void*);
		cback fptr = (cback)Module::symbol_find(name);
		if (fptr) return fptr(params);
		else return NULL;
	}

	inline void * symbol_call(const char * name) {
		typedef void * (*cback)(void);
		cback fptr = (cback)Module::symbol_find(name);
		if (fptr) return fptr();
		else return NULL;
	}
}