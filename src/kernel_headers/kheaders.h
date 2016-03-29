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

#define kprintf(fmt, ...) symbol_call_args_("mod_kprintf", PP_NARG(__VA_ARGS__) + 1, fmt, ##__VA_ARGS__)

#define malloc(bytecount) FCASTF(SYF((char*)"malloc"), void *, uintptr_t)(bytecount)
#define realloc(ptr, bytecount) FCASTF(SYF((char*)"malloc"), void *, void*, uintptr_t)(ptr, bytecount)
#define calloc(nmemb, bytecount) FCASTF(SYF((char*)"malloc"), void *, uintptr_t, uintptr_t)(nmemb, bytecount)
#define valloc(bytecount) FCASTF(SYF((char*)"malloc"), void *, uintptr_t)(bytecount)

#define hashmap_create(count) FCASTF(SYF((char*)"hashmap_create"), hashmap_t *, int)(count)
#define hashmap_set(map, key, value) FCASTF(SYF((char*)"hashmap_set"), void *, hashmap_t *, void *, void *)(map, key, value)
#define hashmap_remove(map, key) FCASTF(SYF((char*)"hashmap_remove"), void *, hashmap_t*, void*)(map, key)
#define hashmap_get_i(map, idx) FCASTF(SYF((char*)"hashmap_get_i"), void *, hashmap_t *, int)(map, idx)
#define hashmap_has(map, key) FCASTF(SYF((char*)"hashmap_has"), int, hashmap_t *, void*)(map, key)

#endif

using namespace Kernel;

#endif /* SRC_KERNEL_HEADERS_KHEADERS_H_ */
