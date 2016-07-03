/*
 * kheaders.h
 *
 *  Created on: 25/03/2016
 *      Author: Miguel
 */

#ifndef SRC_KERNEL_HEADERS_KHEADERS_H_
#define SRC_KERNEL_HEADERS_KHEADERS_H_

#include <numarg.h>
#include <libc.h>
#include <module.h>

#ifdef MODULE

#define syscall_schedule_install(syscall_name, no) FCASTF(SYF("syscall_schedule_install"), char, char*, int, uintptr_t)((char*)# syscall_name, no, (uintptr_t)syscall_name)
#define syscall_uninstall(syscall_addr) FCASTF(SYF("syscall_uninstall"), void, uintptr_t)((uintptr_t)syscall_addr)
#define syscall_run_n(intno) FCASTF(SYF("syscall_run_n"), void, int)(intno)
#define syscall_run_s(syscall_name) FCASTF(SYF("syscall_run_s"), void, char*)(syscall_name)
#define syscall_run_ss(syscall_name) FCASTF(SYF("syscall_run_s"), void, char*)(# syscall_name)

#define is_kernel_init() FCASTF(SYF("is_kernel_init"), char, void)()

#define kprintf(fmt, ...) symbol_call_args_("mod_kprintf", PP_NARG(__VA_ARGS__) + 1, fmt, ##__VA_ARGS__)
#define scwn() SYC("mod_term_scrolldown")
#define scup() SYC("mod_term_scrollup")
#define scbot() SYC("mod_term_scrollbot")
#define sctop() SYC("mod_term_scrolltop")

#define malloc(bytecount) FCASTF(SYF("malloc"), void *, uintptr_t)(bytecount)
#define realloc(ptr, bytecount) FCASTF(SYF("malloc"), void *, void*, uintptr_t)(ptr, bytecount)
#define calloc(nmemb, bytecount) FCASTF(SYF("malloc"), void *, uintptr_t, uintptr_t)(nmemb, bytecount)
#define valloc(bytecount) FCASTF(SYF("malloc"), void *, uintptr_t)(bytecount)

#define free(ptr) FCASTF(SYF("free"), void, void *)(ptr);

#define hashmap_create(count) FCASTF(SYF("hashmap_create"), hashmap_t *, int)(count)
#define hashmap_set(map, key, value) FCASTF(SYF("hashmap_set"), void *, hashmap_t *, void *, void *)(map, key, value)
#define hashmap_remove(map, key) FCASTF(SYF("hashmap_remove"), void *, hashmap_t*, void*)(map, key)
#define hashmap_get_i(map, idx) FCASTF(SYF("hashmap_get_i"), void *, hashmap_t *, int)(map, idx)
#define hashmap_has(map, key) FCASTF(SYF("hashmap_has"), int, hashmap_t *, void*)(map, key)

#define strlen(str) FCASTF(SYF("strlen"), unsigned long, char*)(str)
#define strcpy(dst, src) FCASTF(SYF("strcpy"), char*, char*, char*)(dst, src)

#define IRQ_OFF() SYC("int_disable")
#define IRQ_RES() SYC("int_resume")
#define IRQ_ON()  SYC("int_enable")

#define spin_init(lock)   FCASTF(SYF("spin_init"),   void, spin_lock_t)(lock)
#define spin_lock(lock)   FCASTF(SYF("spin_lock"),   void, spin_lock_t)(lock)
#define spin_unlock(lock) FCASTF(SYF("spin_unlock"), void, spin_lock_t)(lock)

#endif

using namespace Kernel;

#endif /* SRC_KERNEL_HEADERS_KHEADERS_H_ */
