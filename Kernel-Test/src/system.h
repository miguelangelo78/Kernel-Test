#pragma once

#include "console.h"
#include "libc.h"

#define asm __asm__
#define volatile __volatile__

#define DEBUG(msg) term.puts((char*)msg, COLOR_DEFAULT);
#define DEBUGC(msg, color) term.puts((char*)msg, color);
#define DEBUGOK() DEBUGC(" OK \n", COLOR_GOOD);
#define DEBUGVALID() DEBUGC(" VALID \n", COLOR_GOOD);

#define STRSTR(str) #str
#define STR(str) STRSTR(str)
#define ASSERT(cond, msg) { if(!cond) { char buff[256]; sprintf(buff, "Assert (%s): %s", STR(cond), msg); Error::panic(buff, __LINE__, __FILE__, 0); } }

#define KERNEL_PAUSE()   { asm volatile ("hlt"); }
#define KERNEL_FULL_STOP() while (1) { KERNEL_PAUSE(); }

namespace Kernel {
	namespace Error{
		extern void panic(const char * msg, const int line, const char * file, int intno);
		extern void panic(const char * msg, int intno);
		extern void panic(const char * msg);
		extern void panic(void);
		extern void infinite_idle(const char * msg);
	}
}