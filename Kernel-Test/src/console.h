#pragma once

#define VID_WIDTH 80
#define VID_HEIGHT 25

#define VID_CENTER_W VID_WIDTH/2
#define VID_CENTER_H (VID_HEIGHT-1)/2

#define VID_CALC_POS(x, y) (x + y * VID_WIDTH)

enum VIDColor {
	VIDBlack = 0x0,
	VIDBlue = 0x1,
	VIDGreen = 0x2,
	VIDCyan = 0x3,
	VIDRed = 0x4,
	VIDMagenta = 0x5,
	VIDBrown = 0x6,
	VIDLightGray = 0x7,
	VIDDarkGray = 0x8,
	VIDLightBlue = 0x9,
	VIDLightGreen = 0xA,
	VIDLightCyan = 0xB,
	VIDLightRed = 0xC,
	VIDLightMagenta = 0xD,
	VIDYellow = 0xE,
	VIDWhite = 0xF
};

#define COLOR(bg, fg) ((bg<<4) | fg)
#define DEFAULT_COLOR COLOR(VIDBlack, VIDLightGray)

class Console {
	public:
		Console();
		void init(void);
		void putc(char chr, char color);
		void puts(char * str, char color);
		void clear(void);
		void fill(char color);
		void reset_cursor(void);
	private:
		int cursor_x, cursor_y;
		char * vidmem;
};