/*
 * fpu.cpp
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>

static int fpu_init(void) {

	return 0;
}

static int fpu_finit(void) {
	return 0;
}

static uintptr_t fpu_ioctl(void * ioctl_packet) {

	return 0;
}

MODULE_DEF(fpu_driver, fpu_init, fpu_finit, MODT_CLOCK, "Miguel S.", fpu_ioctl);
