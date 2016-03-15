#include <system.h>
#include <stdint.h>
#include <module.h>

/* ISR: Uses the IDT to install and manage exceptions */
/* Reference: http://www.osdever.net/bkerndev/Docs/isrs.htm */

namespace Kernel {
namespace CPU {
namespace ISR {

	/* ISR Messages: */
	const char *exception_msgs[ISR_COUNT] = {
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

	/* Function pointers will be installed here: */
	static isr_handler_t isr_routines[256];

	void isr_install_handler(size_t isrs, isr_handler_t handler) {
		isr_routines[isrs] = handler;
	}

	void isr_uninstall_handler(size_t isrs) {
		isr_routines[isrs] = 0;
	}

	void isrs_install(void) {
		#define ISR_DEFAULT_FLAG 0b10001110 /* Segment Present and in Ring 0 */
		char buffer[16];
		for (int i = 0; i < ISR_COUNT; i++) {
			sprintf(buffer, "_isr%d", i);
			IDT::idt_set_gate(i, (uintptr_t)Module::symbol_find(buffer), SEG_KERNEL_CS, ISR_DEFAULT_FLAG);
		}
		IDT::idt_set_gate(IDT::SYSCALL_VECTOR, (uintptr_t)Module::symbol_find("_isr127"), SEG_KERNEL_CS, ISR_DEFAULT_FLAG);
	}

	void fault_handler(CPU::regs_t * r) { /* Gets called for EVERY ISR and IRQ interrupt */
		isr_handler_t handler = isr_routines[r->int_no];
		if (handler) {
			/* This handler was installed. */
			handler(r);
		}
		else {
			/* Kernel RSOD (aka BSOD) */
			char msg_fmt[256];
			sprintf(msg_fmt, "Fault handler: %s", exception_msgs[r->int_no]);
			Error::panic(msg_fmt, __LINE__, __FILE__, r->int_no);
		}
	}

	extern "C" { void _isr0(void); }
	extern "C" { void _isr1(void); }
	extern "C" { void _isr2(void); }
	extern "C" { void _isr3(void); }
	extern "C" { void _isr4(void); }
	extern "C" { void _isr5(void); }
	extern "C" { void _isr6(void); }
	extern "C" { void _isr7(void); }
	extern "C" { void _isr8(void); }
	extern "C" { void _isr9(void); }
	extern "C" { void _isr10(void); }
	extern "C" { void _isr11(void); }
	extern "C" { void _isr12(void); }
	extern "C" { void _isr13(void); }
	extern "C" { void _isr14(void); }
	extern "C" { void _isr15(void); }
	extern "C" { void _isr16(void); }
	extern "C" { void _isr17(void); }
	extern "C" { void _isr18(void); }
	extern "C" { void _isr19(void); }
	extern "C" { void _isr20(void); }
	extern "C" { void _isr21(void); }
	extern "C" { void _isr22(void); }
	extern "C" { void _isr23(void); }
	extern "C" { void _isr24(void); }
	extern "C" { void _isr25(void); }
	extern "C" { void _isr26(void); }
	extern "C" { void _isr27(void); }
	extern "C" { void _isr28(void); }
	extern "C" { void _isr29(void); }
	extern "C" { void _isr30(void); }
	extern "C" { void _isr31(void); }
	extern "C" { void _isr127(void); }

	EXPORT_SYMBOL(_isr0);
	EXPORT_SYMBOL(_isr1);
	EXPORT_SYMBOL(_isr2);
	EXPORT_SYMBOL(_isr3);
	EXPORT_SYMBOL(_isr4);
	EXPORT_SYMBOL(_isr5);
	EXPORT_SYMBOL(_isr6);
	EXPORT_SYMBOL(_isr7);
	EXPORT_SYMBOL(_isr8);
	EXPORT_SYMBOL(_isr9);
	EXPORT_SYMBOL(_isr10);
	EXPORT_SYMBOL(_isr11);
	EXPORT_SYMBOL(_isr12);
	EXPORT_SYMBOL(_isr13);
	EXPORT_SYMBOL(_isr14);
	EXPORT_SYMBOL(_isr15);
	EXPORT_SYMBOL(_isr16);
	EXPORT_SYMBOL(_isr17);
	EXPORT_SYMBOL(_isr18);
	EXPORT_SYMBOL(_isr19);
	EXPORT_SYMBOL(_isr20);
	EXPORT_SYMBOL(_isr21);
	EXPORT_SYMBOL(_isr22);
	EXPORT_SYMBOL(_isr23);
	EXPORT_SYMBOL(_isr24);
	EXPORT_SYMBOL(_isr25);
	EXPORT_SYMBOL(_isr26);
	EXPORT_SYMBOL(_isr27);
	EXPORT_SYMBOL(_isr28);
	EXPORT_SYMBOL(_isr29);
	EXPORT_SYMBOL(_isr30);
	EXPORT_SYMBOL(_isr31);
	EXPORT_SYMBOL(_isr127);
}
}
}
