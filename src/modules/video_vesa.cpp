/*
 * video_vesa.cpp
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <video.h>

static gfx_t * gfx = 0;
static char is_video_enabled = 0;

static int vid_vesa_init(void) {
	gfx = (gfx_t*)SYF("gfx");
	if(gfx->vid_mode) is_video_enabled = 1;
	SYC("video_finalize");
	return 0;
}

static int vid_vesa_finit(void) {
	return 0;
}

static uintptr_t vid_vesa_ioctl(void * ioctl_packet) {
	if(!is_video_enabled) return IOCTL_NULL;

	for(uint16_t x = 0; x < video_width(); x++) {
		for(uint16_t y = 0; y< video_height(); y++) {
			GFX(x,y) = rgb(x < 255 ? x : y < 255 ? y : y-x ,x < video_width()/2 ? x : y-x, 0);
		}
	}
	return 0;
}

MODULE_DEF(vid_vesa_driver, vid_vesa_init, vid_vesa_finit, MODT_CLOCK, "Miguel S.", vid_vesa_ioctl);
