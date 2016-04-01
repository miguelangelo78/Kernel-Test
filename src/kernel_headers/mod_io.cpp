/*
 * mod_io.cpp
 *
 *  Created on: 26/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <module.h>

void mod_kprintf(char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	if(Log::logging == Log::LOG_VGA)
		Kernel::term.printf(fmt, args, 0);
	else if(Log::logging == Log::LOG_SERIAL)
		Kernel::serial.printf(fmt, args, 0);
	else if(Log::logging == Log::LOG_VGA_SERIAL) {
		Kernel::term.printf(fmt, args, 0);
		Kernel::serial.printf(fmt, args, 0);
	}
	va_end(args);
}
EXPORT_SYMBOL(mod_kprintf);

void mod_term_scrolldown(void) {
	Kernel::term.scroll(1,1);
}
EXPORT_SYMBOL(mod_term_scrolldown);

void mod_term_scrollup(void) {
	Kernel::term.scroll(0,1);
}
EXPORT_SYMBOL(mod_term_scrollup);

void mod_term_scrollbot(void) {
	Kernel::term.scroll_bottom();
}
EXPORT_SYMBOL(mod_term_scrollbot);

void mod_term_scrolltop(void) {
	Kernel::term.scroll_top();
}
EXPORT_SYMBOL(mod_term_scrolltop);


