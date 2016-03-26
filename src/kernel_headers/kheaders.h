/*
 * kheaders.h
 *
 *  Created on: 25/03/2016
 *      Author: Miguel
 */

#ifndef SRC_KERNEL_HEADERS_KHEADERS_H_
#define SRC_KERNEL_HEADERS_KHEADERS_H_

#include <kernel_headers/libc.h>
#include <kernel_headers/numarg.h>

#define kprintf(fmt, ...) symbol_call_args("mod_kprintf", PP_NARG(__VA_ARGS__) + 1, fmt, ##__VA_ARGS__)

#endif /* SRC_KERNEL_HEADERS_KHEADERS_H_ */
