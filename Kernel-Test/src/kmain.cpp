#include "system.h"
#include "module.h"

namespace Kernel {
	Terminal term;
	
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
			/* Reference: http://wiki.osdev.org/Global_Descriptor_Table , http://www.osdever.net/bkerndev/Docs/gdt.htm */
			/* Tutorial: http://wiki.osdev.org/GDT_Tutorial */

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
				} __attribute__((packed)) entries[6]; /* 6 segments */
				
				struct {
					uint16_t limit;
					uintptr_t base;
				} __attribute__((packed)) * pointer;
			//xxx tss_entry_t tss; /* Too early for TSS... */
			} gdt __attribute__((used));

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

				// xxx write_tss(5, 0x10, 0x0); /* Too soon... */

				/* Install GDT and TSS: */
				gdt_flush((uintptr_t)gdt.pointer);
				//xxx tss_flush(); /* Too soon... */
			}
		}

		namespace IDT {
			/* Reference: http://wiki.osdev.org/IDT , http://www.osdever.net/bkerndev/Docs/idt.htm */

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
	
			/* Actual IDT Table: */
			struct {
				struct {
					uint16_t base_low;
					uint16_t sel; /* Segment selector */
					uint8_t zero; /* This field must always be 0 */
					uint8_t flags; /* Attributes for a certain entry (seg. present and ring lvl) */
					uint16_t base_high;
				} __attribute__((packed)) entries[256];
			
				struct {
					uint16_t limit;
					uintptr_t base;
				} __attribute__((packed)) * pointer;
			} idt __attribute__((used));

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
	
		namespace ISR { /* ISR: Uses the IDT to install and manage exceptions */
			/* Reference: http://www.osdever.net/bkerndev/Docs/isrs.htm */
			
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

			/* Function callback type for IRQs: */
			typedef void(*irq_handler_t) (CPU::regs_t *);
			
			/* Function pointers will be installed here: */
			static irq_handler_t isr_routines[256];

			void isr_install_handler(size_t isrs, irq_handler_t handler) {
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
				irq_handler_t handler = isr_routines[r->int_no];
				if (handler) {
					/* This handler was installed. This is used for Keyboard, or any other IO device. */
					handler(r);
				} else { 
					/* Kernel RSOD (aka BSOD) */
					char msg_fmt[256];
					sprintf(msg_fmt, "Fault handler: %s", exception_msgs[r->int_no]);
					Error::panic(msg_fmt, __LINE__, __FILE__, r->int_no);
				}
			}
		}

		namespace IRQ { /* IRQ: Uses the IDT to install and manage interrupt requests */
			/* Reference:	http://www.osdever.net/bkerndev/Docs/irqs.htm ,
							http://wiki.osdev.org/PIC */
			
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
			#define ICW1_ICW4 0x01
			#define ICW1_INIT 0x10

			/* Interrupt Request constants: */
			#define IRQ_COUNT 16
			#define IRQ_CHAIN_DEPTH 4
			#define IRQ_OFFSET 32
			#define SYNC_CLI() asm volatile("cli") /* Disables interrupts */
			#define SYNC_STI() asm volatile("sti") /* Enables interrupts */

			/* Callback constants: */
			typedef int(*irq_handler_chain_t) (CPU::regs_t *); /* Callback type */
			static irq_handler_chain_t irq_routines[IRQ_COUNT * IRQ_CHAIN_DEPTH] = { 0 }; /* IRQ Callback handlers */
			
			/* Interrupt functions used by external modules: */
			static volatile int sync_depth = 0; /* Used by interrupts */
			inline void int_disable(void) {
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
			
			void int_enable(void) {
				sync_depth = 0;
				SYNC_STI();
			}

			void int_resume(void) {
				/* If there is one or no call depths, reenable interrupts */
				if (sync_depth == 0 || sync_depth == 1) SYNC_STI();
				else sync_depth--;
			}

			void irq_install_handler(size_t irq_num, irq_handler_chain_t irq_handler) {
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

			void irq_uninstall_handler(size_t irq_num) {
				/* Disable interrupts when changing handlers */
				SYNC_CLI();
				for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++)
					irq_routines[i * IRQ_COUNT + irq_num] = 0;
				SYNC_STI();
			}

			inline void irq_ack(size_t irq_num) {
				if (irq_num >= 8)
					IO::outb(PIC2_CMD, PIC_EOI);
				IO::outb(PIC1_CMD, PIC_EOI);
			}

			/* Implementation and initialization functions: */
			#define irq_is_valid(int_no) ((int_no) >= IRQ_OFFSET && (int_no) <= IRQ_OFFSET + (IRQ_COUNT - 1)) /* IRQ_COUNT - 1 because it starts from 0 */
	
			void irq_handler(CPU::regs_t * r) {
				/* Disable interrupts when handling */
				int_disable();
				if (irq_is_valid(r->int_no)) {
					for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
						irq_handler_chain_t handler = irq_routines[i * IRQ_COUNT + (r->int_no - IRQ_OFFSET)];
						/* Check and run irq handler: */
						if (handler && handler(r))
							goto done;
					}
					irq_ack(r->int_no - IRQ_OFFSET);
				}
			done:
				int_resume();
			}

			inline void pic8259_init(void) {
				using namespace IO;
				/* Cascade initialization */
				outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4); PIC_WAIT();
				outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4); PIC_WAIT();

				/* Remap */
				outb(PIC1_DATA, PIC1_OFFSET); PIC_WAIT();
				outb(PIC2_DATA, PIC2_OFFSET); PIC_WAIT();

				/* Cascade identity with slave PIC at IRQ2 */
				outb(PIC1_DATA, 0x04); PIC_WAIT();
				outb(PIC2_DATA, 0x02); PIC_WAIT();

				/* Request 8086 mode on each PIC */
				outb(PIC1_DATA, 0x01); PIC_WAIT();
				outb(PIC2_DATA, 0x01); PIC_WAIT();
			}

			void irq_install(void) {
				#define IRQ_DEFAULT_FLAG 0b10001110 /* Segment Present and in Ring 0 */
				/* Install IRQs' address onto IDT: */
				char buffer[16];
				for (size_t i = 0; i < IRQ_COUNT; i++) {
					sprintf(buffer, "_irq%d", i);
					IDT::idt_set_gate(IRQ_OFFSET + i, (uintptr_t)Module::symbol_find(buffer), SEG_KERNEL_CS, IRQ_DEFAULT_FLAG);
				}
				pic8259_init(); /* Initialize the 8259 PIC */
			}
		}
	}

	/* Initialization data goes here, like the Multiboot, for example */
	namespace Init {
		#define MULTIBOOT_HEADER_MAGIC 0x2BADB002

		#define MULTIBOOT_FLAG_MEM     0x001
		#define MULTIBOOT_FLAG_DEVICE  0x002
		#define MULTIBOOT_FLAG_CMDLINE 0x004
		#define MULTIBOOT_FLAG_MODS    0x008
		#define MULTIBOOT_FLAG_AOUT    0x010
		#define MULTIBOOT_FLAG_ELF     0x020
		#define MULTIBOOT_FLAG_MMAP    0x040
		#define MULTIBOOT_FLAG_CONFIG  0x080
		#define MULTIBOOT_FLAG_LOADER  0x100
		#define MULTIBOOT_FLAG_APM     0x200
		#define MULTIBOOT_FLAG_VBE     0x400

		/*!
		* \struct multiboot_t
		* \brief Multiboot Header Structure
		*
		* When a Multiboot compliant bootloader load our kernel,
		* it inits a structure and store it in the EBX register.
		* In ksharp_stage2.asm we pass 3 arguments to the main function :
		* this structure, the multiboot magic number and the initial
		* stack.
		*/
		struct multiboot_t
		{
			uint32_t flags;
			uint32_t mem_lower;
			uint32_t mem_upper;
			uint32_t boot_device;
			uint32_t cmdline;
			uint32_t mods_count;
			uint32_t mods_addr;

			struct
			{
				uint32_t num;
				uint32_t size;
				uint32_t addr;
				uint32_t shndx;
			} elf_sec;

			uint32_t mmap_length;
			uint32_t mmap_addr;
			uint32_t drives_length;
			uint32_t drives_addr;
			uint32_t config_table;
			uint32_t boot_loader_name;
			uint32_t apm_table;

			struct
			{
				uint32_t control_info;
				uint32_t mode_info;
				uint32_t mode;
				uint32_t interface_seg;
				uint32_t interface_off;
				uint32_t interface_len;
			} vbe;
		} __attribute__((packed));
	}

	int kmain(struct Init::multiboot_t * mboot, unsigned magic, uint32_t initial_stack) 
	{
		/******* Initialize everything: *******/
		
		term.init();
	
		/* Validate Multiboot: */
		DEBUG(">> Initializing Kernel <<\n\n> Checking Multiboot...");
		ASSERT(magic == MULTIBOOT_HEADER_MAGIC, "Multiboot is not valid!");
		DEBUGVALID();
		
		/* Install GDT: */
		DEBUG("> Installing GDT - ");
		CPU::GDT::gdt_init();
		DEBUGOK();

		/* Install IDT: */
		DEBUG("> Installing IDT - ");
		CPU::IDT::idt_init();
		DEBUGOK();

		/* Install ISRs: */
		DEBUG("> Installing ISRs - ");
		CPU::ISR::isrs_install();
		DEBUGOK();

		/* Install IRQs: */
		DEBUG("> Installing IRQs (PIC) - ");
		CPU::IRQ::irq_install();
		DEBUGOK();
		
		/* All done! */
		DEBUGC("\nReady", COLOR_GOOD);
		for(;;);

		return 0;
	}

	void kexit()
	{
		Error::infinite_idle("!! The Kernel has exited !!");
	}
}
