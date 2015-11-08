#pragma once

#include "system.h"

namespace Module {
	extern "C" { void kernel_symbols_start(void); }
	extern "C" { void kernel_symbols_end(void); }

	typedef struct {
		uintptr_t addr;
		char name[];
	} kernel_sym_t;

	inline void * symbol_find(const char * name) {
		kernel_sym_t * k = (kernel_sym_t *)&kernel_symbols_start;
		while ((uintptr_t)k < (uintptr_t)&kernel_symbols_end) {
			if (strcmp(k->name, name)) {
				k = (kernel_sym_t *)((uintptr_t)k + sizeof *k + strlen(k->name) + 1);
				continue;
			}
			return (void*)k->addr;
		}
		return NULL;
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