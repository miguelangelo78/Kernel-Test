#pragma once

#include <va_list.h>
#include <numarg.h>
#include <stdint.h>

#define IOCTL_NULL ((uintptr_t)-1)

/********** MODULES **********/
enum MODULE_CATEGORIES {
	MODT_UNKOWN_TYPE, MODT_CLOCK, MODT_PS2, MODT_STORAGE, MODT_PNP, MODT_ACPI, MODT_IO, MODT_AUDIO, MODT_VIDEO, MODT_MULTIMEDIA, MODT_FS, MODT_NET, MODT_DEBUG, MODT_SYS, MODT_OTHER
};

enum MODULE_SCHEDULE_MODE {
	MODULE_SCHED_QUICK, MODULE_SCHED_LATE
};

#define MODULE_SIGNATURE0 modent_
#define MODULE_SIGNATURE1 '_'
#define MODULE_SIGNATUREXT0 modentext_

#define MOD_IOCTL(modname, ...) do{ uintptr_t ioctl_packet[] = {__VA_ARGS__}; module_ioctl(modname, ioctl_packet); } while(0)
#define MOD_IOCTLD(modname, retvar, ...) do{ uintptr_t ioctl_packet[] = {__VA_ARGS__}; retvar = module_ioctl(modname, ioctl_packet); } while(0)

#define MODULE_DEF(...) GET_MACRO(__VA_ARGS__, MODDEF_6, MODDEF_5, MODDEF_4, MODDEF_3)(__VA_ARGS__)

#define MODDEF_3(name, ini, fini) modent_t modent_ ## name = { #name, ini, fini , 0, 0, ""}
#define MODDEF_4(name, ini, fini, modtype) modent_t modent_ ## name = { #name, ini, fini , 0, modtype, ""}
#define MODDEF_5(name, ini, fini, modtype, author) modent_t modent_ ## name = { #name, ini, fini , 0, modtype, author}
#define MODDEF_6(name, ini, fini, modtype, author, ioctl) modent_t modent_ ## name = { #name, ini, fini , ioctl, modtype, author}
#define MODULE_EXT(name) modent_t modentext_ ## name = { #name, 0, 0 }

#ifdef MODULE
#define module_get(modname) FCASTF(SYF("module_gets"), modent_t*, char*)(modname)
#define module_geti(modidx) FCASTF(SYF("module_geti"), modent_t*, int)(modidx)
#define module_count() FCASTF(SYF("module_count"), modent_t*, void)()
#define module_type_exists(modtype) FCASTF(SYF("module_type_exists"), modent_t*, char)(modtype)
#define module_exists(modname) FCASTF(SYF("module_exists"), modent_t*, char*)(modname)
#define module_ioctl(modname, data) FCASTF(SYF("module_ioctl_s"), uintptr_t, char*, void*)(modname, data)
#define module_ioctli(modidx, data) FCASTF(SYF("module_ioctl_i"), uintptr_t, int, void*)(modidx, data)
#define module_schedule_quick(wait_for_module, run_address) FCASTF(SYF("module_schedule"), char, char, char*, uintptr_t)(MODULE_SCHED_QUICK, wait_for_module, (uintptr_t)&run_address)
#define module_schedule(run_address) FCASTF(SYF("module_schedule"), char, char, char*, uintptr_t)(MODULE_SCHED_LATE, "", (uintptr_t)&run_address)
#endif

/* This macro is only used by the genmake script */
#define MODULE_DEPS(...)

typedef struct {
	char name[23];
	int (*init)(void);
	int (*finit)(void);
	uintptr_t (*ioctl)(void *);
	char type;
	char author[16];
} modent_t;

enum MOD_TYPE {
	MOD_UNKNOWN, MOD_CORE, MOD_DEP
};

/********** SYMBOLS **********/
#define KERNEL_SYMBOLS_TABLE_START 0x100000 /* Very important macro!! */
/* We reserved 2 pages for the symbol table at the very start of the kernel: */
#define KERNEL_SYMBOLS_TABLE_SIZE (0x1000 * 2) /* Each entry on the table is 8 bytes. This means the number of symbols that we can export is: TABLE_SIZE (in KiB) / 8 bytes  */
#define KERNEL_SYMBOLS_TABLE_END (KERNEL_SYMBOLS_TABLE_START + KERNEL_SYMBOLS_TABLE_SIZE) /* Very important macro!! */

#ifndef MODULE
#define EXPORT_SYMBOL(sym) \
	sym_t sym_## sym __attribute__((section(".symbols"))) = {(char*)#sym, (uintptr_t)&sym}
#else
#define EXPORT_SYMBOL(sym) \
	sym_t sym_## sym = {"modsym_" #sym, (uintptr_t)&sym}
#endif

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

/* All invalid symbols will point here: */
static inline int symbol_invalid(void) {
	return -1;
}

static inline void * symbol_find(char * name, char get_addr) {
	static sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
	for(unsigned int i = 0; i < KERNEL_SYMBOLS_TABLE_SIZE / sizeof(sym_t); i++)
		if(!strcmp(sym[i].name, name))
			return get_addr ? (void*)sym[i].addr : &sym[i];
	/* Symbol not found: */
	return (void*)&symbol_invalid;
}

static inline void * symbol_find(char * name) {
	return symbol_find(name, 1);
}

static inline sym_t * symbol_t_find(char * name) {
	return (sym_t*)symbol_find(name, 0);
}

#define MAX_ARGUMENT 10

#define symbol_call_args(function_name, ...) symbol_call_args_((char*)#function_name, PP_NARG(__VA_ARGS__), __VA_ARGS__)

#define SYF(symbol_name) symbol_find(symbol_name)
#define SYA(function_name, ...) symbol_call_args(function_name, __VA_ARGS__)
#define SYC(function_name) symbol_call(function_name)

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
	for(unsigned int i = 0; i < KERNEL_SYMBOLS_TABLE_SIZE / sizeof(sym_t); i++)
		if(sym[i].name) symcount++;
	return symcount;
}

/* Finds the next available symbol slot: */
static inline sym_t * symbol_next(void) {
	static sym_t * sym = (sym_t *)KERNEL_SYMBOLS_TABLE_START;
	for(unsigned int i = 0; i < KERNEL_SYMBOLS_TABLE_SIZE / sizeof(sym_t); i++)
		if(!sym[i].name)
			return &sym[i];
	/* Symbol not found: */
	return (sym_t*)0;
}

static inline char symbol_exists(char * name) {
	sym_t * sym = symbol_t_find(name);
	return sym && sym != (sym_t*)&symbol_invalid ? (sym->name ? 1 : 0) : 0;
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
	return SYM(0);
}

#ifdef __cplusplus
#ifndef MODULE

#include <system.h>
#include <stdint.h>

namespace Module {
	/********** MODULES **********/
	extern void modules_load(void);
	extern modent_t * module_get(char * modname);
	extern modent_t * module_get(int mod_idx);
	extern uintptr_t module_ioctl(char * modname, void * data);
	extern uintptr_t module_ioctl(int mod_idx, void * data);
	extern int module_count(void);
	extern char module_type_exists(char mod_type);
	extern char module_exists(char * modname);
	extern char module_exists(modent_t * mod);
	extern int module_add(modent_t * mod);
	extern char module_remove(char * modname);
	extern char module_remove(int mod_idx);
	extern char module_remove(modent_t * mod);

	extern "C" { void kernel_symbols_start(void); }
	extern "C" { void kernel_symbols_end(void); }

	static inline void symbols_dump(int start, int end) {
		/* Only the kernel can call this function */
		int symcount = symbol_count();
		int begin = (start >= 0 && start < KERNEL_SYMBOLS_TABLE_END && (start <= end || end==-1)) ? start : 0;
		int finish = (end <= KERNEL_SYMBOLS_TABLE_END && end > 0 && end >= start) ? end : symcount;

		kprintf(">>>>> Dumping Symbol Table\n>> Section Start: 0x%x - End: 0x%x Size: 0x%x\n>> Symbol count: %d",
			KERNEL_SYMBOLS_TABLE_START,
			KERNEL_SYMBOLS_TABLE_END,
			(uintptr_t)KERNEL_SYMBOLS_TABLE_END - (uintptr_t)KERNEL_SYMBOLS_TABLE_START, symcount);
		kprintf(" - Showing: Start = %d | End = %d\n___________\n", begin, finish);

		sym_t * sym = SYM(0);
		for(int i = begin; i < finish; i++)
			kprintf("%d - %s: 0x%x\n", i+1, sym[i].name, sym[i].addr);
		kprintf("___________\n>> Dump done\n");
	}

	static inline void symbols_dump_start(int start) {
		symbols_dump(start, -1);
	}

	static inline void symbols_dump(int end) {
		symbols_dump(-1, end);
	}

	static inline void symbols_dump(void) {
		symbols_dump(-1, -1);
	}
}

using namespace Module;

#endif
#endif
