#pragma once

typedef unsigned char uint8_t;

#define COLOR(bg, fg) ((bg<<4) | fg)

class Console {
	public:
		Console();
		void putc(char chr, char color);
		void puts(char * str, char color);
		void clear();
		void reset_cursor();

	private:
		int cursor_x, cursor_y;
		uint8_t * vidmem;
};