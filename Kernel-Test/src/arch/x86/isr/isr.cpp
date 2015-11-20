#include <system.h>
#include <stdint.h>
#include <module.h>

/* ISR: Uses the IDT to install and manage exceptions */
/* Reference: http://www.osdever.net/bkerndev/Docs/isrs.htm */

namespace Kernel {
namespace CPU {
namespace ISR {

	/* Function pointers will be installed here: */
	static isr_handler_t isr_routines[256];

	void isr_install_handler(size_t isrs, isr_handler_t handler) {
		isr_routines[isrs] = handler;
	}

	void isr_uninstall_handler(size_t isrs) {
		isr_routines[isrs] = 0;
	}

	void __init isrs_install(void) {
		#define ISR_DEFAULT_FLAG 0b10001110 /* Segment Present and in Ring 0 */
		char buffer[16];
		for (int i = 0; i < ISR_COUNT; i++) {
			sprintf(buffer, "_isr%d", i);
			IDT::idt_set_gate(i, (uintptr_t)Module::symbol_find(buffer), SEG_KERNEL_CS, ISR_DEFAULT_FLAG);
		}
		IDT::idt_set_gate(IDT::SYSCALL_VECTOR, (uintptr_t)Module::symbol_find("_isr127"), SEG_KERNEL_CS, ISR_DEFAULT_FLAG);
	}

	void __interrupt fault_handler(CPU::regs_t * r) { /* Gets called for EVERY ISR and IRQ interrupt */
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
}
}
}
