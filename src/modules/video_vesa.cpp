/*
 * video_vesa.cpp
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>

static int vid_vesa_init(void) {

	return 0;
}

static int vid_vesa_finit(void) {
	return 0;
}

static uintptr_t vid_vesa_ioctl(void * ioctl_packet) {

	return 0;
}

MODULE_DEF(vid_vesa_driver, vid_vesa_init, vid_vesa_finit, MODT_CLOCK, "Miguel S.", vid_vesa_ioctl);
