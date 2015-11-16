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
#define ASSERT(cond, msg) { if(!(cond)) { char buff[256]; sprintf(buff, "Assert (%s): %s", STR(cond), msg); Error::panic(buff, __LINE__, __FILE__, 0); } }

#define IRQ_OFF() CPU::IRQ::int_disable()
#define IRQ_RES() CPU::IRQ::int_resume()
#define IRQ_ON() CPU::IRQ::int_enable()

#define KERNEL_PAUSE() { asm volatile ("hlt"); }
#define KERNEL_FULL_PAUSE() while (1) { KERNEL_PAUSE(); }

#define KERNEL_FULL_STOP() while(1) { IRQ_OFF(); KERNEL_FULL_PAUSE(); }

namespace Kernel {
	namespace CPU {
		namespace IRQ {
			void int_disable(void);
			void int_enable(void);
			void int_resume(void);
		}
	}

	namespace Error{
		extern void panic(const char * msg, const int line, const char * file, int intno);
		extern void panic(const char * msg, int intno);
		extern void panic(const char * msg);
		extern void panic(void);
		extern void infinite_idle(const char * msg);
	}
}