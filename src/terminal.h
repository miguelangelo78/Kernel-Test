#pragma once

#include <va_list.h>

#define VID_WIDTH 80
#define VID_HEIGHT 25

#define VID_CENTER_W VID_WIDTH/2
#define VID_CENTER_H (VID_HEIGHT-1)/2

#define VID_CALC_POS(x, y) (x + y * VID_WIDTH)

enum VIDColor {
	VIDBlack,
	VIDBlue,
	VIDGreen,
	VIDCyan,
	VIDRed,
	VIDMagenta,
	VIDBrown,
	VIDLightGray,
	VIDDarkGray,
	VIDLightBlue,
	VIDLightGreen,
	VIDLightCyan,
	VIDLightRed,
	VIDLightMagenta,
	VIDYellow,
	VIDWhite
};

#define COLOR(bg, fg) ((bg<<4) | fg)
#define COLOR_DEFAULT COLOR(VIDBlack, VIDLightGray)
#define COLOR_GOOD COLOR(VIDGreen, VIDWhite)
#define COLOR_BAD COLOR(VIDRed, VIDWhite)
#define COLOR_WARNING COLOR(VIDYellow, VIDBlack)
#define COLOR_INFO COLOR(VIDBlue, VIDWhite)

class Terminal {
	public:
		Terminal();
		void init(void);
		void putc(const char chr, uint8_t color);
		void putc(const char chr);
		void puts(const char * str, uint8_t color);
		void puts(const char * str);
		void printf(uint8_t color, const char *fmt, ...);
		void printf(uint8_t color, const char *fmt, va_list args);
		void printf(const char *fmt, ...);
		void printf(const char *fmt, va_list args, char ign);
		void clear(void);
		void fill(uint8_t color);
		void reset_cursor(void);
		Point go_to(uint8_t x, uint8_t y);
	private:
		void hide_textmode_cursor(void);
		int cursor_x, cursor_y;
		char * vidmem;
};
