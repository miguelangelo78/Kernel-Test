#pragma once

#include <kinit.h>
#include <terminal.h>
#include <libc.h>
#include <io.h>
#include <attr.h>

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

	/* All CPU Related components, such as GDT,
	IDT (which includes ISR and PIC / IRQ) and registers */
	namespace CPU {
		typedef struct {
			unsigned int gs, fs, es, ds;
			unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
			unsigned int int_no, err_code;
			unsigned int eip, cs, eflags, useresp, ss;
		} regs_t;

		namespace GDT {
			void __init gdt_init(void);
			void gdt_set_gate(uint8_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran);
		}

		namespace IDT {
			/* IDT Interrupt List: */
			enum IDT_IVT {
				ISR_DIVBY0,
				ISR_RESERVED0,
				ISR_NMI,
				ISR_BREAK,
				ISR_OVERFLOW,
				ISR_BOUNDS,
				ISR_INVOPCODE,
				ISR_DEVICEUN,
				ISR_DOUBLEFAULT,
				ISR_COPROC,
				ISR_INVTSS,
				ISR_SEG_FAULT,
				ISR_STACKSEG_FAULT,
				ISR_GENERALPROT,
				ISR_PAGEFAULT,
				ISR_RESERVED1,
				ISR_FPU,
				ISR_ALIGNCHECK,
				ISR_SIMD_FPU,
				ISR_RESERVED2,
				ISR_USR,
				SYSCALL_VECTOR = 0x7F
			};

			void __init idt_init();
			void idt_set_gate(uint8_t num, uintptr_t isr_addr, uint16_t sel, uint8_t flags);
		}

		namespace ISR {
			#define ISR_COUNT 32

			/* ISR Messages: */
			static const char *exception_msgs[ISR_COUNT] = {
				"Division by zero",
				"Debug",
				"Non-maskable interrupt",
				"Breakpoint",
				"Detected overflow",
				"Out-of-bounds",
				"Invalid opcode",
				"No coprocessor",
				"Double fault",
				"Coprocessor segment overrun",
				"Bad TSS",
				"Segment not present",
				"Stack fault",
				"General protection fault",
				"Page fault",
				"Unknown interrupt",
				"Coprocessor fault",
				"Alignment check",
				"Machine check",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved",
				"Reserved"
			};

			/* Function callback type for ISRs: */
			typedef void(*isr_handler_t) (CPU::regs_t *);

			void __init isrs_install(void);
			void isr_install_handler(size_t isrs, isr_handler_t handler);
			void isr_uninstall_handler(size_t isrs);
		}

		namespace IRQ {
			typedef int(*irq_handler_t) (CPU::regs_t *); /* Callback type */

			void irq_install(void);

			void int_disable(void);
			void int_enable(void);
			void int_resume(void);

			void irq_install_handler(size_t irq_num, irq_handler_t irq_handler);
			void irq_uninstall_handler(size_t irq_num);
			void irq_ack(size_t irq_num);
			void irq_mask(uint8_t irq_num, uint8_t enable);
			void irq_set_mask(uint8_t irq_num);
			void irq_clear_mask(uint8_t irq_num);
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
