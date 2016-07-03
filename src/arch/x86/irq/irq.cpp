#include <system.h>
#include <module.h>

/* IRQ: Uses the IDT to install and manage interrupt requests */
/* Reference:	http://www.osdever.net/bkerndev/Docs/irqs.htm ,
				http://wiki.osdev.org/PIC ,
				https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf 
*/

namespace Kernel {
namespace CPU {
namespace IRQ {

	/* 8259 Programmable Interrupt Controller (8259 PIC) constants: */
	/* PIC 1: */
	#define PIC1_ADDR 0x20 /* IO base address for master PIC */
	#define PIC1_CMD PIC1_ADDR
	#define PIC1_OFFSET 0x20
	#define PIC1_DATA (PIC1_ADDR + 1)
							/* PIC 2: */
	#define PIC2_ADDR 0xA0 /* IO base address for slave PIC */
	#define PIC2_CMD PIC2_ADDR
	#define PIC2_OFFSET 0x28
	#define PIC2_DATA (PIC2_ADDR + 1)
							/* End-of-interrupt command code */
	#define PIC_EOI 0x20
	#define PIC_WAIT() \
					do { \
						asm volatile("jmp 1f\n\t" \
										"1:\n\t" \
										"    jmp 2f\n\t" \
										"2:"); \
					} while (0)

	/* Initialization constants: */
	#define ICW1_ICW4 0x01 /* ICW4 (not) needed */
	#define ICW1_SINGLE	0x02 /* Single (cascade) mode */
	#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
	#define ICW1_LEVEL 0x08 /* Level triggered (edge) mode */
	#define ICW1_INIT 0x10 /* Initialization - required! */

	#define ICW4_8086 0x01 /* 8086/88 (MCS-80/85) mode */
	#define ICW4_AUTO 0x02 /* Auto (normal) EOI */
	#define ICW4_BUF_SLAVE	0x08 /* Buffered mode/slave */
	#define ICW4_BUF_MASTER	0x0C /* Buffered mode/master */
	#define ICW4_SFNM 0x10 /* Special fully nested (not) */

	/* Interrupt Request constants: */
	#define IRQ_COUNT 16 /* How many IRQ lines exist (8 for each PIC, only 15 usable) */
	#define IRQ_CHAIN_DEPTH 4
	#define IRQ_OFFSET 32 /* Offset which separates ISRs from IRQs on the IDT entry table */
	#define SYNC_CLI() asm volatile("cli") /* Disables interrupts */
	#define SYNC_STI() asm volatile("sti") /* Enables interrupts */

	/* Just helper macros, so we can understand what they do: */
	#define pic_send_cmd(pic_cmd_addr, cmd) IO::outb(pic_cmd_addr, cmd); PIC_WAIT();
	#define pic_send_dat(pic_dat_addr, dat) IO::outb(pic_dat_addr, dat); PIC_WAIT();

	/* IRQ Callback handlers */
	static irq_handler_t irq_routines[IRQ_COUNT * IRQ_CHAIN_DEPTH] = { 0 };

	/* Interrupt functions used by external modules: */
	static volatile int sync_depth = 0; /* Used by interrupts */

	void int_disable(void) {
		/* Check if interrupts are enabled */
		uint32_t flags;
		asm volatile("pushf\n\t"
			"pop %%eax\n\t"
			"movl %%eax, %0\n\t"
			: "=r"(flags)
			:
			: "%eax");

		/* Disable interrupts */
		SYNC_CLI();

		/* If interrupts were enabled, then this is the first call depth */
		if (flags & (1 << 9))
			sync_depth = 1;
		else /* Otherwise there is now an additional call depth */
			sync_depth++;
	}
	EXPORT_SYMBOL(int_disable);

	void int_enable(void) {
		sync_depth = 0;
		SYNC_STI();
	}
	EXPORT_SYMBOL(int_enable);

	void int_resume(void) {
		/* If there is one or no call depths, reenable interrupts */
		if (sync_depth == 0 || sync_depth == 1) SYNC_STI();
		else sync_depth--;
	}
	EXPORT_SYMBOL(int_resume);

	void irq_install_handler(size_t irq_num, irq_handler_t irq_handler) {
		/* Disable interrupts when changing handlers */
		SYNC_CLI();
		for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
			if (irq_routines[i * IRQ_COUNT + irq_num])
				continue;
			irq_routines[i * IRQ_COUNT + irq_num] = irq_handler;
			break;
		}
		SYNC_STI();
	}
	EXPORT_SYMBOL(irq_install_handler);

	void irq_uninstall_handler(size_t irq_num) {
		/* Disable interrupts when changing handlers */
		SYNC_CLI();
		for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++)
			irq_routines[i * IRQ_COUNT + irq_num] = 0;
		SYNC_STI();
	}

	void irq_ack(size_t irq_num) {
		if (irq_num >= 8) IO::outb(PIC2_CMD, PIC_EOI);
		IO::outb(PIC1_CMD, PIC_EOI);
	}

	/* Implementation and initialization functions: */
	#define irq_is_valid(int_no) ((int_no) >= IRQ_OFFSET && (int_no) <= IRQ_OFFSET + (IRQ_COUNT - 1)) /* IRQ_COUNT - 1 because it starts from 0 */

	void irq_handler(CPU::regs_t * r) {
		/* Disable interrupts when handling */
		int_disable();
		if (irq_is_valid(r->int_no)) {
			for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
				irq_handler_t handler = irq_routines[i * IRQ_COUNT + (r->int_no - IRQ_OFFSET)];
				/* Check and run irq handler: */
				if (handler && handler(r))
					goto done;
			}
			irq_ack(r->int_no - IRQ_OFFSET);
		}
	done:
		irq_ack(r->int_no - IRQ_OFFSET);
		int_resume();
	}

	void irq_mask(uint8_t irq_num, uint8_t enable) {
		uint16_t port;
		if (irq_num < 8) {
			port = PIC1_DATA;
		}
		else {
			port = PIC2_DATA;
			irq_num -= 8;
		}

		if (enable)
			IO::outb(port, IO::inb(port) | (1 << irq_num));
		else
			IO::outb(port, IO::inb(port) & ~(1 << irq_num));
	}

	void irq_set_mask(uint8_t irq_num) {
		irq_mask(irq_num, 1);
	}

	void irq_clear_mask(uint8_t irq_num) {
		irq_mask(irq_num, 0);
	}

	void pic8259_init(void) {
		/* Cascade initialization */
		pic_send_cmd(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
		pic_send_cmd(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

		/* Remap */
		pic_send_dat(PIC1_DATA, PIC1_OFFSET);
		pic_send_dat(PIC2_DATA, PIC2_OFFSET);

		/* Cascade identity with slave PIC at IRQ2 */
		pic_send_dat(PIC1_DATA, ICW1_INTERVAL4);
		pic_send_dat(PIC2_DATA, ICW1_SINGLE);

		/* Request 8086 mode on each PIC */
		pic_send_dat(PIC1_DATA, ICW4_8086);
		pic_send_dat(PIC2_DATA, ICW4_8086);
	}

	void irq_install(void) {
		#define IRQ_DEFAULT_FLAG 0b10001110 /* Segment Present and in Ring 0 */
		/* Install IRQs' address onto IDT: */
		char buffer[16];
		for (size_t i = 0; i < IRQ_COUNT; i++) {
			sprintf(buffer, "_irq%d", i);
			IDT::idt_set_gate(IRQ_OFFSET + i, (uintptr_t)symbol_find(buffer), SEG_KERNEL_CS, IRQ_DEFAULT_FLAG);
		}
		pic8259_init(); /* Initialize the 8259 PIC */
	}

	extern "C" { void _irq0(void); }
	extern "C" { void _irq1(void); }
	extern "C" { void _irq2(void); }
	extern "C" { void _irq3(void); }
	extern "C" { void _irq4(void); }
	extern "C" { void _irq5(void); }
	extern "C" { void _irq6(void); }
	extern "C" { void _irq7(void); }
	extern "C" { void _irq8(void); }
	extern "C" { void _irq9(void); }
	extern "C" { void _irq10(void); }
	extern "C" { void _irq11(void); }
	extern "C" { void _irq12(void); }
	extern "C" { void _irq13(void); }
	extern "C" { void _irq14(void); }
	extern "C" { void _irq15(void); }

	EXPORT_SYMBOL(_irq0);
	EXPORT_SYMBOL(_irq1);
	EXPORT_SYMBOL(_irq2);
	EXPORT_SYMBOL(_irq3);
	EXPORT_SYMBOL(_irq4);
	EXPORT_SYMBOL(_irq5);
	EXPORT_SYMBOL(_irq6);
	EXPORT_SYMBOL(_irq7);
	EXPORT_SYMBOL(_irq8);
	EXPORT_SYMBOL(_irq9);
	EXPORT_SYMBOL(_irq10);
	EXPORT_SYMBOL(_irq11);
	EXPORT_SYMBOL(_irq12);
	EXPORT_SYMBOL(_irq13);
	EXPORT_SYMBOL(_irq14);
	EXPORT_SYMBOL(_irq15);
}
}
}
