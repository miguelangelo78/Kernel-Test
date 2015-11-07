#include "console.h"

Console::Console() { }

void Console::init() {
	cursor_x = cursor_y = 0;
	vidmem = (char*)0xB8000;
	clear();
}

void Console::putc(const char chr, char color) {
	if(chr=='\n') { cursor_y++; cursor_x = 0; return; }
	int loc = VID_CALC_POS(cursor_x, cursor_y);
	vidmem[loc * 2] = chr;
	vidmem[loc * 2 + 1] = color;
	cursor_x ++;
}

void Console::puts(const char * str, char color) {
	while (*str) putc(*str++, color);
}

void Console::clear() {
	reset_cursor();
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(VIDBlack, VIDBlack));
	reset_cursor();
}

void Console::fill(char bgcolor) {
	reset_cursor();
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(bgcolor, VIDBlack));
	reset_cursor();
}

void Console::reset_cursor() {
	cursor_x = cursor_y = 0;
}