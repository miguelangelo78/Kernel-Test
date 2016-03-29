/*
 * cmos.cpp
 *
 *  Created on: 29/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>

static int cmos_init(void) {
	return 0;
}

static int cmos_finit(void) {
	return 0;
}

static uintptr_t cmos_ioctl(void * data) {

	return 0;
}

MODULE_DEF(cmos_driver, cmos_init, cmos_finit, MODT_CLOCK, "Miguel S.", cmos_ioctl);
