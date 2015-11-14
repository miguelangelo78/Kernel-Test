#pragma once

#include "terminal.h"
#include "libc.h"
#include "io.h"

/* Memory segment selectors: */
enum SEGSEL {
	SEG_NULL,
	SEG_KERNEL_CS = 0x8,
	SEG_KERNEL_DS = 0x10,
	SEG_USER_CS = 0x18,
	SEG_USER_DS = 0x20
};

#define asm __asm__
#define volatile __volatile__

#define DEBUG(msg) term.puts((char*)msg, COLOR_DEFAULT);
#define DEBUGC(msg, color) term.puts((char*)msg, color);
#define DEBUGOK() DEBUGC(" OK \n", COLOR_GOOD);
#define DEBUGVALID() DEBUGC(" VALID \n", COLOR_GOOD);

#define STRSTR(str) #str
#define STR(str) STRSTR(str)
#define ASSERT(cond, msg) { if(!cond) { char buff[256]; sprintf(buff, "Assert (%s): %s", STR(cond), msg); Error::panic(buff, __LINE__, __FILE__, 0); } }

#define IRQ_OFF // TODO

#define KERNEL_PAUSE()   { asm volatile ("hlt"); }
#define KERNEL_FULL_PAUSE() while (1) { KERNEL_PAUSE(); }

namespace Kernel {
	namespace Error{
		extern void panic(const char * msg, const int line, const char * file, int intno);
		extern void panic(const char * msg, int intno);
		extern void panic(const char * msg);
		extern void panic(void);
		extern void infinite_idle(const char * msg);
	}
}