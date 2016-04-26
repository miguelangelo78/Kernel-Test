#include <system.h>
#include <stdint.h>

/* Reference: http://wiki.osdev.org/Global_Descriptor_Table , http://www.osdever.net/bkerndev/Docs/gdt.htm */
/* Tutorial: http://wiki.osdev.org/GDT_Tutorial */

namespace Kernel {
namespace CPU {
namespace TSS { /* Forward TSS declaration */
	void write_tss(int32_t num, uint16_t ss0, uint32_t esp0);
	void tss_flush(void);
}
namespace GDT {
	/* Declared in arch/x86/gdt/gdt_flush.s: */
	extern "C" { void gdt_flush(uintptr_t); }

	/* Actual GDT Table: */
	static struct {
		struct {
			/* Limits */
			uint16_t limit_low;
			/* Segment address */
			uint16_t base_low;
			uint8_t base_middle;
			/* Access modes */
			uint8_t access; /* Contains: Present, Ring (0=lvl 0, 3=lvl 3), is Exec., Segment Grow Dir, RW and Accessed bit */
			uint8_t granularity; /* Contains: Granularity (1Byte/4KiB) and Mode (0 = 16 bit Real Mode, 1 =32 bit Protected Mode) */
			uint8_t base_high;
		} __packed entries[6]; /* 6 segments */

		struct {
			uint16_t limit;
			uintptr_t base;
		} __packed * pointer;
		CPU::TSS::tss_entry_t tss;
	} gdt;

	void gdt_set_gate(uint8_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran) {
		/* Base Address */
		gdt.entries[num].base_low = (base & 0xFFFF);
		gdt.entries[num].base_middle = (base >> 16) & 0xFF;
		gdt.entries[num].base_high = (base >> 24) & 0xFF;
		/* Limits */
		gdt.entries[num].limit_low = (limit & 0xFFFF);
		gdt.entries[num].granularity = (limit >> 16) & 0X0F;
		/* Granularity */
		gdt.entries[num].granularity |= (gran & 0xF0);
		/* Access flags */
		gdt.entries[num].access = access;
	}

	void gdt_init(void) {
		/* Set up GDT pointer: */
		gdt.pointer->limit = sizeof gdt.entries - 1;
		gdt.pointer->base = (uintptr_t)&gdt.entries[0];

		gdt_set_gate(0, 0, 0, 0, 0);                /* NULL segment (seg. selector: 0x00) */
		gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment (kernel's code, seg. selector: 0x08) */
		gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment (seg. selector: 0x10) */
		gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User code (seg. selector: 0x18) */
		gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User data (seg. selector: 0x20) */
		CPU::TSS::write_tss(5, 0x10, 0x0);

		/* Install GDT and TSS: */
		gdt_flush((uintptr_t)gdt.pointer);
		CPU::TSS::tss_flush();
	}
}
namespace TSS {
	void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
		tss_entry_t * tss = &CPU::GDT::gdt.tss;
		uintptr_t base = (uintptr_t)tss;
		uintptr_t limit = base + sizeof *tss;

		/* Add the TSS descriptor to the GDT */
		CPU::GDT::gdt_set_gate(num, base, limit, 0xE9, 0x00);

		memset(tss, 0x0, sizeof *tss);

		tss->ss0 = ss0;
		tss->esp0 = esp0;
		tss->cs = 0x0b;
		tss->ss = 0x13;
		tss->ds = 0x13;
		tss->es = 0x13;
		tss->fs = 0x13;
		tss->gs = 0x13;

		tss->iomap_base = sizeof *tss;
	}

	void tss_set_kernel_stack(uintptr_t stack) {
		/* Set the kernel stack */
		CPU::GDT::gdt.tss.esp0 = stack;
	}

	void tss_flush(void) {
		asm volatile ("mov $0x2B, %ax; ltr %ax");
	}
}
}
}
