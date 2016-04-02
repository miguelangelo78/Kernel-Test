/*
 * video.h
 *
 *  Created on: 01/04/2016
 *      Author: Miguel
 */

#ifndef SRC_VIDEO_H_
#define SRC_VIDEO_H_

#include <stdint.h>

typedef struct {
	uint16_t vid_mode;
	uint16_t width, height, depth;
	char * vidmem;
} gfx_t;

enum VID_MODES {
	VID_269 = 0, VID_270 = 0, VID_271 = 0,
	VID_256 = 1, VID_257 = 2, VID_272 = 2,
	VID_273 = 2, VID_274 = 2, VID_258 = 3,
	VID_106 = 3, VID_259 = 3, VID_275 = 3,
	VID_276 = 3, VID_277 = 3, VID_260 = 4,
	VID_261 = 4, VID_278 = 4, VID_279 = 4,
	VID_280 = 4, VID_262 = 5, VID_263 = 5,
	VID_281 = 5, VID_282 = 5, VID_283 = 5
};

#ifndef MODULE
extern gfx_t * gfx;
extern void video_init(int mode);
extern char * video_mode_get(int mode);
extern uint16_t video_width();
extern uint16_t video_height();
extern uint16_t video_palette(void);
#else
#define video_width() (gfx->width)
#define video_height() (gfx->height)
#define video_palette() (gfx->depth)
#endif

/****** GRAPHICS: *******/
#define VID (gfx->vidmem)
#define CTX_W() (gfx->width)
#define CTX_H() (gfx->height)
#define CTX_B() ((gfx->depth / 8))

#define GFX(x, y) *((uint32_t*)&(VID[(CTX_W()*(y) + (x)) * CTX_B()]))

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
	return (r * 0x10000) + (g * 0x100) + (b * 0x1);
}

static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a * 0x1000000) + (r * 0x10000) + (g * 0x100) + b;
}

#endif /* SRC_VIDEO_H_ */
