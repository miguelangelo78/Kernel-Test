#pragma once

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

class Console {
	public:
		Console();
		void init(void);
		void putc(const char chr, char color);
		void puts(const char * str, char color);
		void clear(void);
		void fill(char color);
		void reset_cursor(void);
	private:
		int cursor_x, cursor_y;
		char * vidmem;
};