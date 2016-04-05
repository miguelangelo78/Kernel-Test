#pragma once

#include <va_list.h>
#include "terminal_font.h"

#define VID_WIDTH 80
#define VID_WIDTH_TOTAL (VID_WIDTH * 2)
#define VID_HEIGHT 25
#define VID_MEM_TOTAL VID_WIDTH_TOTAL * VID_HEIGHT

#define VID_CENTER_W VID_WIDTH/2
#define VID_CENTER_H (VID_HEIGHT-1)/2

#define VID_CALC_POS(x, y) (x + y * VID_WIDTH)

#define TERMINAL_CURSOR ' '
#define TERMINAL_TEXTMODE_CURSOR 219

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
#define COLOR_DEFAULT gfx->vid_mode ? 0xBABABA : COLOR(VIDBlack, VIDLightGray)
#define COLOR_GOOD gfx->vid_mode ? 0x1CBA1F : COLOR(VIDGreen, VIDWhite)
#define COLOR_BAD COLOR(VIDRed, VIDWhite)
#define COLOR_WARNING COLOR(VIDYellow, VIDBlack)
#define COLOR_INFO gfx->vid_mode ? 0x6071DB : COLOR(VIDBlue, VIDWhite)
#define COLOR_INIT_HEADER gfx->vid_mode ? 0xE2E831 : COLOR(VIDBlack, VIDYellow)

class Terminal {
	public:
		Terminal();
		void init(int gfx_mode);
		void init(void);
		void putc(const char chr, uint32_t color);
		void putc(const char chr, uint32_t color, uint32_t bgcolor);
		void putc(const char chr);
		void puts(const char * str, uint32_t color);
		void puts(const char * str, uint32_t color, uint32_t bgcolor);
		void puts(const char * str);
		void printf(uint32_t color, const char *fmt, ...);
		void printf(uint32_t color, const char *fmt, va_list args);
		void printf(const char *fmt, ...);
		void printf(const char *fmt, va_list args, char ign);
		char scroll(char direction, int scrollcount);
		void scroll_copy_line(char * buff, int line_dst, int line_src);
		char * scroll_copy_line(char * buff, char * buff2, int line_dst, int line_src);
		void scroll_copy_line(int line_dst, int line_src);
		char * scroll_copy_line(char* buff, int line);
		char * scroll_copy_line(int line);
		void scroll_bottom(void);
		void scroll_top(void);
		void clear(void);
		void fill(uint32_t color);
		void reset_cursor(void);
		Point go_to(uint32_t x, uint32_t y);
	private:
		void draw_cursor(char redraw, uint32_t bgcolor);
		void spill_buff(char spill_direction);
		void fill_buff(char fill_direction);
		void putc_textmode(const char chr, uint32_t color);
		void putc_gfx(const char chr, uint32_t color, uint32_t bgcolor);
		void hide_textmode_cursor(void);
		void last_line_store(int line, int last_pos);
		void last_lines_shiftup(void);
		int last_line_get_lastpos(int line);
		int cursor_x, cursor_y;
		int scroll_y, scroll_y_orig;
		int gfx_mode;
		char * term_buffer;
		int * line_lastchar;
		int line_lastchar_size;
};
