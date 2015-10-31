#include "console.h"

Console::Console() : cursor_x(0), cursor_y(0), vidmem((uint8_t*)0xB8000) { }

void Console::putc(char chr, char color) {
	vidmem[cursor_x] = chr;
	vidmem[cursor_x + 1] = color;
	cursor_x += 2;
}

void Console::puts(char * str, char color) {
	while (*str) putc(*str++, color);
}

void Console::clear() {
	for (int i = 0; i<2000; i++)
		putc(' ', COLOR(0x1, 0));
	reset_cursor();
}

void Console::reset_cursor() {
	cursor_x = cursor_y = 0;
}