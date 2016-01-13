#include <system.h>

/* Reference: http://wiki.osdev.org/IDT , http://www.osdever.net/bkerndev/Docs/idt.htm */

namespace Kernel {
namespace CPU {
namespace IDT {
	/* Actual IDT Table: */
	struct {
		struct {
			uint16_t base_low;
			uint16_t sel; /* Segment selector */
			uint8_t zero; /* This field must always be 0 */
			uint8_t flags; /* Attributes for a certain entry (seg. present and ring lvl) */
			uint16_t base_high;
		} __packed entries[256];

		struct {
			uint16_t limit;
			uintptr_t base;
		} __packed * pointer;
	} idt;

	void idt_set_gate(uint8_t num, uintptr_t isr_addr, uint16_t sel, uint8_t flags) {
		idt.entries[num].base_low = ((uintptr_t)isr_addr & 0xFFFF); /* Mask low 16 bit (low half) */
		idt.entries[num].base_high = ((uintptr_t)isr_addr >> 16) & 0xFFFF; /* Mask high 16 bit (higher half) */
		idt.entries[num].sel = sel;
		idt.entries[num].zero = 0;
		idt.entries[num].flags = flags | 0x60;
	}

	#define idt_flush(idt_ptr) asm volatile("lidtl (%0)" : : "r"(idt_ptr));

	void idt_init() {
		/* Set up IDT pointer: */
		idt.pointer->limit = sizeof idt.entries - 1;
		idt.pointer->base = (uintptr_t)&idt.entries[0];

		/* Install IDT: */
		idt_flush(idt.pointer);
	}
}
}
}