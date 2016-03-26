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
	if(Log::logging == LOG_VGA)
		Kernel::term.printf(fmt, args, 0);
	else if(Log::logging == LOG_SERIAL)
		Kernel::serial.printf(fmt, args, 0);
	va_end(args);
}

EXPORT_SYMBOL(mod_kprintf);

