#include <system.h>
#include <video.h>

using namespace Kernel::IO;

Terminal::Terminal() {
	gfx_mode = 0;
	cursor_y = cursor_x = scroll_y = scroll_y_orig = 0;
	term_buffer = 0;
	gfx_line_lastchar = 0;
}

void Terminal::init(int gfx_mode) {
	this->gfx_mode = gfx_mode;
	if(!gfx_mode) {
		#define term_buffer_size VID_MEM_TOTAL * 10
		term_buffer = (char*)malloc(term_buffer_size); /* Video memory will spill to this buffer */
		memset(term_buffer, 0, term_buffer_size);
	} else {
		gfx_line_lastchar = (int*)malloc(sizeof(int) * (FONT_CHARS_PERCOLUMN));
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

void Terminal::putc_textmode(const char chr, uint32_t color) {
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

void gfx_char(char c, int x, int y, uint32_t color, uint32_t bg_color, char transparent) {
	for(int h = 0; h < FONT_HEIGHT; h++) {
		for(int w = 0, _w = FONT_WIDTH; w < FONT_WIDTH; w++, _w--)
			if((FONT_CHAR(c)[h] & (1 << _w)))
				GFX(w + x, h + y) = color;
			else if(!transparent)
				GFX(w + x, h + y) = bg_color;
	}
}

void Terminal::draw_cursor(char redraw, uint32_t bgcolor) {
	if(gfx_mode) { /* Graphics mode */
		if(redraw == 1) { /* Remove cursor from newlines */
			gfx_char(' ', cursor_x*FONT_W_PADDING, (cursor_y-1)*FONT_H_PADDING, bgcolor, bgcolor, 0);
			gfx_char(TERMINAL_CURSOR, 0, cursor_y*FONT_H_PADDING, FONT_DEFAULT_COLOR, bgcolor, 0);
		} else { /* Draw cursor normally */
			gfx_char(TERMINAL_CURSOR, cursor_x*FONT_W_PADDING, cursor_y*FONT_H_PADDING, FONT_DEFAULT_COLOR, bgcolor, 0);
		}
	} else { /* Text mode */

	}
}

void Terminal::last_line_store(int line, int last_pos) {
	if(!gfx_mode) return;
	gfx_line_lastchar[line] = last_pos;
}

void Terminal::last_lines_shiftup(void) {
	if(!gfx_mode) return;
	for(int i=0;i<FONT_CHARS_PERCOLUMN-1;i++)
		gfx_line_lastchar[i] = gfx_line_lastchar[i+1];
	gfx_line_lastchar[FONT_CHARS_PERCOLUMN] = 0;
}

int Terminal::last_line_get_lastpos(int line) {
	return gfx_line_lastchar[line];
}

void Terminal::putc_gfx(const char chr, uint32_t color, uint32_t bgcolor) {
	/* New line: */
	if(chr == '\n' || chr == '\r') {
		last_line_store(cursor_y++, cursor_x);
		draw_cursor(1, bgcolor);
		if(chr=='\n') cursor_x = 0;
		if(cursor_y >= FONT_CHARS_PERCOLUMN) {
			cursor_y--;
			scroll_y_orig++;
			scroll(1, 1);
		}
		return;
	}

	/* Backspace: */
	if(chr == 8) {
		if(!cursor_x || cursor_x-1 < 0) {
			gfx_char(' ', (cursor_x)*FONT_W_PADDING, cursor_y * FONT_H_PADDING, 0, bgcolor, 0);
			cursor_x = last_line_get_lastpos(--cursor_y);
		} else {
			cursor_x--;
			gfx_char(' ', (cursor_x+1)*FONT_W_PADDING, cursor_y*FONT_H_PADDING, 0, bgcolor, 0);
		}
		gfx_char(TERMINAL_CURSOR, (cursor_x)*FONT_W_PADDING, cursor_y*FONT_H_PADDING, FONT_DEFAULT_COLOR, 0, 0);
		return;
	}

	/* Draw character with default font: */
	gfx_char(chr, cursor_x * FONT_W_PADDING, cursor_y * FONT_H_PADDING, chr == ' ' ? bgcolor : color, bgcolor, 0);

	if(++cursor_x >= FONT_CHARS_PERLINE) {
		last_line_store(cursor_y++, cursor_x-1);
		cursor_x = 0;
		if(cursor_y >= FONT_CHARS_PERCOLUMN) {
			cursor_y--;
			scroll_y_orig++;
			scroll(1,1);
		}
	}
	draw_cursor(0, bgcolor);
}

void Terminal::putc(const char chr, uint32_t color, uint32_t bgcolor) {
	if(gfx_mode) putc_gfx(chr, color, bgcolor);
	else putc_textmode(chr, color);
}

void Terminal::putc(const char chr, uint32_t color) {
	putc(chr, color, 0);
}

void Terminal::putc(const char chr) {
	putc(chr, gfx_mode ? FONT_DEFAULT_COLOR : COLOR(VIDBlack, VIDLightGray));
}

void Terminal::puts(const char * str, uint32_t color) {
	while (*str) putc(*str++, color);
}

void Terminal::puts(const char * str, uint32_t color, uint32_t bgcolor) {
	while (*str) putc(*str++, color, bgcolor);
}

void Terminal::puts(const char * str) {
	while (*str) putc(*str++, gfx_mode ? FONT_DEFAULT_COLOR : COLOR(VIDBlack, VIDLightGray));
}

void Terminal::scroll_copy_line(char * buff, int line_dst, int line_src) {
	uint16_t vid_width_total = gfx_mode ? gfx->width * 3 : VID_WIDTH_TOTAL;
	memcpy(buff + vid_width_total * line_dst, buff + vid_width_total * (line_src), vid_width_total);
}

char * Terminal::scroll_copy_line(char * buff, char * buff2, int line_dst, int line_src) {
	uint16_t vid_width_total = gfx_mode ? gfx->width * 3 : VID_WIDTH_TOTAL;
	memcpy(buff + vid_width_total * line_dst, buff2 + vid_width_total * (line_src), vid_width_total);
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
			last_lines_shiftup();
			if(gfx_mode) {
				for(int j = 0; j < FONT_HEIGHT;j++) {
					for(int line = 0; line <= gfx->height; line++)
						scroll_copy_line(line, line + 1);
					uint16_t vid_width_total = gfx->width * 3;
					memset(gfx->vidmem + vid_width_total * (gfx->height-1), 0, vid_width_total);
				}
			}
			else {
				for(int line = 0; line < gfx->height; line++)
					scroll_copy_line(line, line + 1);
			}
		} else { /* Scroll up */
			if(scroll_y == 1 || gfx_mode) return 0; /* No scrolling up in graphics mode */
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
	if(gfx_mode) return; /* In graphics mode we don't spill into any buffer */
	if(spill_direction) /* Spill on top of screen */
		scroll_copy_line(term_buffer, gfx->vidmem, scroll_y - 1, 0);
	else /* Spill on bottom of screen */
		scroll_copy_line(term_buffer, gfx->vidmem, VID_HEIGHT + scroll_y - 1, VID_HEIGHT - 1);
}

void Terminal::fill_buff(char fill_direction) {
	if(gfx_mode) return; /* In graphics mode we don't fill from any buffer */
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

void Terminal::fill(uint32_t bgcolor) {
	reset_cursor();
	if(gfx->vid_mode) {
		for(int x = 0; x < gfx->width; x++)
			for(int y = 0; y < gfx->height; y++)
				GFX(x,y) = bgcolor;
	} else {
		for (int i = 0; i < VID_WIDTH * VID_HEIGHT; i++)
			putc(' ', COLOR(bgcolor, VIDBlack));
		reset_cursor();
	}
}

void Terminal::reset_cursor() {
	cursor_x = cursor_y = 0;
}

Point Terminal::go_to(uint32_t x, uint32_t y) {
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
							
void Terminal::printf(uint32_t color, const char *fmt, ...) {
	term_printf(fmt, color);
}

void Terminal::printf(const char *fmt, ...) {
	term_printf(fmt, gfx_mode ? FONT_DEFAULT_COLOR : COLOR(VIDBlack, VIDLightGray));
}

void Terminal::printf(uint32_t color, const char *fmt, va_list args) {
	vasprintf(printf_buff, fmt, args);
	puts(printf_buff, color);
}

void Terminal::printf(const char *fmt, va_list args, char ign) {
	vasprintf(printf_buff, fmt, args);
	puts(printf_buff, gfx_mode ? FONT_DEFAULT_COLOR : COLOR(VIDBlack, VIDLightGray));
}
