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

#define kprintf(fmt, ...) symbol_call_args_("mod_kprintf", PP_NARG(__VA_ARGS__) + 1, fmt, ##__VA_ARGS__)

#include <module.h>

#endif /* SRC_KERNEL_HEADERS_KHEADERS_H_ */
