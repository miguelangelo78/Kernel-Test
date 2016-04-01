#include <system.h>
#include <video.h>

using namespace Kernel::IO;

Terminal::Terminal() { gfx_mode = 0; cursor_y = cursor_x = scroll_y = 0; term_buffer = 0; }

void Terminal::init(int gfx_mode) {
	this->gfx_mode = gfx_mode;
	if(!gfx_mode) {
		#define term_buffer_size VID_MEM_TOTAL * 10
		term_buffer = (char*)malloc(term_buffer_size); /* Video memory will spill to this buffer */
		memset(term_buffer, 0, term_buffer_size);
	}
	cursor_x = cursor_y = 0;
	hide_textmode_cursor();
	clear();
}

void Terminal::init(void) {
	init(0);
}

void Terminal::hide_textmode_cursor() {
	outb(0x3D4, 14);
	outb(0x3D5, 0xFF);
	outb(0x3D4, 15);
	outb(0x3D5, 0xFF);
}

void Terminal::putc_textmode(const char chr, uint8_t color) {
	if(scroll_y != scroll_y_orig) scroll_bottom();

	/* New line: */
	if(chr == '\n' || chr == '\r') {
		cursor_y++;
		if(chr=='\n') cursor_x = 0;
		if(cursor_y >= gfx->height) {
			cursor_y--;
			scroll_y_orig++;
			scroll(1, 1);
		}
		return;
	}

	/* Normal character: */
	int loc = VID_CALC_POS(cursor_x, cursor_y) * 2;
	VID[loc] = chr;
	VID[loc + 1] = color;

	if(++cursor_x >= gfx->width) {
		cursor_x = 0;
		if(++cursor_y >= gfx->height) {
			cursor_y--;
			scroll_y_orig++;
			scroll(1,1);
		}
	}
}

void Terminal::putc_gfx(const char chr, uint8_t color) {
	/* Draw character with default font: */

}

void Terminal::putc(const char chr, uint8_t color) {
	if(gfx_mode) putc_gfx(chr, color);
	else putc_textmode(chr, color);
}

void Terminal::putc(const char chr) {
	putc(chr, COLOR_DEFAULT);
}

void Terminal::puts(const char * str, uint8_t color) {
	while (*str) putc(*str++, color);
}

void Terminal::puts(const char * str) {
	while (*str) putc(*str++, COLOR_DEFAULT);
}

void Terminal::scroll_copy_line(char * buff, int line_dst, int line_src) {
	memcpy(buff + VID_WIDTH_TOTAL * line_dst, buff + VID_WIDTH_TOTAL * (line_src), VID_WIDTH_TOTAL);
}

char * Terminal::scroll_copy_line(char * buff, char * buff2, int line_dst, int line_src) {
	memcpy(buff + VID_WIDTH_TOTAL * line_dst, buff2 + VID_WIDTH_TOTAL * (line_src), VID_WIDTH_TOTAL);
	return buff;
}

void Terminal::scroll_copy_line(int line_dst, int line_src) {
	scroll_copy_line(gfx->vidmem, line_dst, line_src);
}

char line_copy[VID_WIDTH_TOTAL];
char * Terminal::scroll_copy_line(char* buff, int line) {
	memcpy(line_copy, buff + VID_WIDTH_TOTAL * (line), VID_WIDTH_TOTAL);
	return line_copy;
}

char * Terminal::scroll_copy_line(int line) {
	scroll_copy_line(gfx->vidmem, line);
	return line_copy;
}

char Terminal::scroll(char direction, int scrollcount) {
	for(int i = 0; i < scrollcount; i++) {
		if(direction) { /* Scroll down */
			spill_buff(direction);
			if(scroll_y == scroll_y_orig) return 0;
			scroll_y++;
			for(int line = 0; line < gfx->height; line++)
				scroll_copy_line(line, line + 1);
		} else { /* Scroll up */
			if(scroll_y==1) return 0;
			spill_buff(direction);
			scroll_y--;
			for(int line = gfx->height - 1; line >= 0; line--)
				scroll_copy_line(line, line - 1);
		}
		fill_buff(direction);
	}
	return 1;
}

void Terminal::spill_buff(char spill_direction) {
	if(spill_direction) /* Spill on top of screen */
		scroll_copy_line(term_buffer, gfx->vidmem, scroll_y - 1, 0);
	else /* Spill on bottom of screen */
		scroll_copy_line(term_buffer, gfx->vidmem, VID_HEIGHT + scroll_y - 1, VID_HEIGHT - 1);
}

void Terminal::fill_buff(char fill_direction) {
	if(!fill_direction)	/* Fill on top of screen */
		scroll_copy_line(gfx->vidmem, term_buffer, 0, scroll_y - 1);
	else /* Fill on bottom of screen */
		scroll_copy_line(gfx->vidmem, term_buffer, VID_HEIGHT - 1, VID_HEIGHT + scroll_y - 1);
}

void Terminal::scroll_bottom(void) {
	while(scroll(1,1));
}

void Terminal::scroll_top(void) {
	scroll(0, scroll_y);
}

void Terminal::clear() {
	reset_cursor();
	for (int i = 0; i < VID_WIDTH * VID_HEIGHT; i++)
		putc(' ', COLOR(VIDBlack, VIDBlack));
	reset_cursor();
}

void Terminal::fill(uint8_t bgcolor) {
	reset_cursor();
	for (int i = 0; i < VID_WIDTH * VID_HEIGHT; i++)
		putc(' ', COLOR(bgcolor, VIDBlack));
	reset_cursor();
}

void Terminal::reset_cursor() {
	cursor_x = cursor_y = 0;
}

Point Terminal::go_to(uint8_t x, uint8_t y) {
	Point oldp;
	oldp.X = cursor_x;
	oldp.Y = cursor_y;
	cursor_x = x;
	cursor_y = y;
	return oldp;
}

char printf_buff[256];
#define term_printf(fmt, color) strfmt(printf_buff, fmt); \
								puts(printf_buff, color); \
							
void Terminal::printf(uint8_t color, const char *fmt, ...) {
	term_printf(fmt, color);
}

void Terminal::printf(const char *fmt, ...) {
	term_printf(fmt, COLOR_DEFAULT);
}

void Terminal::printf(uint8_t color, const char *fmt, va_list args) {
	vasprintf(printf_buff, fmt, args);
	puts(printf_buff, color);
}

void Terminal::printf(const char *fmt, va_list args, char ign) {
	vasprintf(printf_buff, fmt, args);
	puts(printf_buff, COLOR_DEFAULT);
}
