#include "terminal.h"
#include "IO.h"

using namespace Kernel::IO;

Terminal::Terminal() { }

void Terminal::init() {
	cursor_x = cursor_y = 0;
	vidmem = (char*)0xB8000;
	hide_textmode_cursor();
	clear();
}

void Terminal::hide_textmode_cursor() {
	outb(0x3D4, 14);
	outb(0x3D5, 0xFF);
	outb(0x3D4, 15);
	outb(0x3D5, 0xFF);
}

void Terminal::putc(const char chr, char color) {
	if(chr=='\n') { cursor_y++; cursor_x = 0; return; }
	int loc = VID_CALC_POS(cursor_x, cursor_y);
	vidmem[loc * 2] = chr;
	vidmem[loc * 2 + 1] = color;
	cursor_x ++;
}

void Terminal::puts(const char * str, char color) {
	while (*str) putc(*str++, color);
}

void Terminal::clear() {
	reset_cursor();
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(VIDBlack, VIDBlack));
	reset_cursor();
}

void Terminal::fill(char bgcolor) {
	reset_cursor();
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(bgcolor, VIDBlack));
	reset_cursor();
}

void Terminal::reset_cursor() {
	cursor_x = cursor_y = 0;
}