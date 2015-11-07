#include "console.h"
#include "stdint.h"

void * memset(void * dest, int c, size_t n) {
	uint8_t *ptr = (uint8_t*)dest;
	for(size_t i = 0; i < n; i++)
		ptr[i] = (uint8_t)c;
	return dest;
}

namespace Kernel {
	#define asm __asm__
	#define volatile __volatile__

	#define DEBUG(msg) term.puts((char*)msg, DEFAULT_COLOR);
	#define DEBUGC(msg, color) term.puts((char*)msg, color);

	#define KERNEL_STOP() for(;;) { asm("cli"); asm("hlt"); }

	Console term; /* Used only for debugging to the screen */

	namespace Error {
		void panic(const char * msg, int errcode) {
			term.fill(VIDRed);
			term.puts((char*)"!! KERNEL PANIC !!\n\n - ", COLOR(VIDRed, VIDWhite));
			term.puts(msg, COLOR(VIDRed, VIDWhite));
			KERNEL_STOP();
		}
	}

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
			extern "C" { void gdt_flush(uintptr_t); }

			typedef struct {
				/* Limits */
				uint16_t limit_low;
				/* Segment address */
				uint16_t base_low;
				uint8_t base_middle;
				/* Access modes */
				uint8_t access;
				uint8_t granularity;
				uint8_t base_high;
			} __attribute__((packed)) gdt_entry_t;

			typedef struct {
				uint16_t limit;
				uintptr_t base;
			} __attribute__((packed)) gdt_pointer_t;

			static struct {
				gdt_entry_t entries[6];
				gdt_pointer_t pointer;
			//xxx tss_entry_t tss;
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
				gdt_pointer_t *gdtp = &gdt.pointer;
				gdtp->limit = sizeof gdt.entries - 1;
				gdtp->base = (uintptr_t)&gdt.entries[0];

				gdt_set_gate(0, 0, 0, 0, 0);                /* NULL segment */
				gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment */
				gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment */
				gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* 	User code 	*/
				gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* 	User data 	*/

				// xxx write_tss(5, 0x10, 0x0);

				/* Go go go */
				gdt_flush((uintptr_t)gdtp);
				//xxx tss_flush();
			}
		}

		namespace IDT {
			typedef void(*idt_gate_t)(void);

			extern "C" { void idt_flush(uintptr_t); }

			typedef struct {
				uint16_t base_low;
				uint16_t sel;
				uint8_t zero;
				uint8_t flags;
				uint16_t base_high;
			} __attribute__((packed)) idt_entry_t;

			typedef struct {
				uint16_t limit;
				uintptr_t base;
			} __attribute__((packed)) idt_pointer_t;

			static struct {
				idt_entry_t entries[256];
				idt_pointer_t pointer;
			} idt __attribute__((used));

			void idt_set_gate(uint8_t num, idt_gate_t isr_addr, uint16_t sel, uint8_t flags) {
				idt.entries[num].base_low = ((uintptr_t)isr_addr & 0xFFFF);
				idt.entries[num].base_high = ((uintptr_t)isr_addr >> 16) & 0xFFFF;
				idt.entries[num].sel = sel;
				idt.entries[num].zero = 0;
				idt.entries[num].flags = flags | 0x60;
			}

			void idt_init() {
				/* Set up IDT pointer: */
				idt_pointer_t * idt_ptr = &idt.pointer;
				idt_ptr->limit = sizeof idt.entries - 1;
				idt_ptr->base = (uintptr_t)&idt.entries[0];
				memset(&idt.entries[0], 0 , sizeof idt.entries);

				/* Install IDT: */
				idt_flush((uintptr_t)idt_ptr);
			}
		}
	
		namespace ISR { /* ISR: Used in exceptions */
			#define ISR_COUNT 32

			/* ISR routines declared in isr_defs.s: */
			#pragma region ISRS_ASM_DECL
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
			extern "C" { void irq0(void); }
			extern "C" { void irq1(void); }
			extern "C" { void irq2(void); }
			extern "C" { void irq3(void); }
			extern "C" { void irq4(void); }
			extern "C" { void irq5(void); }
			extern "C" { void irq6(void); }
			extern "C" { void irq7(void); }
			extern "C" { void irq8(void); }
			extern "C" { void irq9(void); }
			extern "C" { void irq10(void); }
			extern "C" { void irq11(void); }
			extern "C" { void irq12(void); }
			extern "C" { void irq13(void); }
			extern "C" { void irq14(void); }
			extern "C" { void irq15(void); }
			extern "C" { void _isr128(void); }
			#pragma endregion

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

			/* ISR List: */
			enum ISR_IVT {
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
				ISR_USR
			};

			typedef void(*irq_handler_t) (CPU::regs_t *);

			static struct {
				size_t index;
				void (*stub)(void);
			} isrs[32 + 1] __attribute__((used));

			static irq_handler_t isr_routines[256];

			void isrs_install(void) {
				IDT::idt_set_gate(0x00, _isr0, 0x08, 0x8E);
				IDT::idt_set_gate(0x01, _isr1, 0x08, 0x8E);
				IDT::idt_set_gate(0x02, _isr2, 0x08, 0x8E);
				IDT::idt_set_gate(0x03, _isr3, 0x08, 0x8E);
				IDT::idt_set_gate(0x04, _isr4, 0x08, 0x8E);
				IDT::idt_set_gate(0x05, _isr5, 0x08, 0x8E);
				IDT::idt_set_gate(0x06, _isr6, 0x08, 0x8E);
				IDT::idt_set_gate(0x07, _isr7, 0x08, 0x8E);
				IDT::idt_set_gate(0x08, _isr8, 0x08, 0x8E);
				IDT::idt_set_gate(0x09, _isr9, 0x08, 0x8E);
				IDT::idt_set_gate(0x0A, _isr10, 0x08, 0x8E);
				IDT::idt_set_gate(0x0B, _isr11, 0x08, 0x8E);
				IDT::idt_set_gate(0x0C, _isr12, 0x08, 0x8E);
				IDT::idt_set_gate(0x0D, _isr13, 0x08, 0x8E);
				IDT::idt_set_gate(0x0E, _isr14, 0x08, 0x8E);
				IDT::idt_set_gate(0x0F, _isr15, 0x08, 0x8E);
				IDT::idt_set_gate(0x10, _isr16, 0x08, 0x8E);
				IDT::idt_set_gate(0x11, _isr17, 0x08, 0x8E);
				IDT::idt_set_gate(0x12, _isr18, 0x08, 0x8E);
				IDT::idt_set_gate(0x13, _isr19, 0x08, 0x8E);
				IDT::idt_set_gate(0x14, _isr20, 0x08, 0x8E);
				IDT::idt_set_gate(0x15, _isr21, 0x08, 0x8E);
				IDT::idt_set_gate(0x16, _isr22, 0x08, 0x8E);
				IDT::idt_set_gate(0x17, _isr23, 0x08, 0x8E);
				IDT::idt_set_gate(0x18, _isr24, 0x08, 0x8E);
				IDT::idt_set_gate(0x19, _isr25, 0x08, 0x8E);
				IDT::idt_set_gate(0x1A, _isr26, 0x08, 0x8E);
				IDT::idt_set_gate(0x1B, _isr27, 0x08, 0x8E);
				IDT::idt_set_gate(0x1C, _isr28, 0x08, 0x8E);
				IDT::idt_set_gate(0x1D, _isr29, 0x08, 0x8E);
				IDT::idt_set_gate(0x1E, _isr30, 0x08, 0x8E);
				IDT::idt_set_gate(0x1F, _isr31, 0x08, 0x8E);
			}
			
			void foo(int){}
			
			void fault_handler(CPU::regs_t * r) {
			foo(1);
				irq_handler_t handler = isr_routines[r->int_no];
				if (handler) {
					handler(r);
				} else {
					// Kernel BSOD
					Error::panic(exception_msgs[r->err_code], r->err_code);
				}
			}
		}

		namespace IRQ {

		}

	}

	/* Initialization data goes here, like the Multiboot, for example: */
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
		
		DEBUG(">> Initializing Kernel <<\n\n");
	
		DEBUG("> Checking Multiboot...");
		if (magic != MULTIBOOT_HEADER_MAGIC) {
			/* TODO: ADD PANIC */
			term.fill(VIDRed);
			DEBUGC("ERROR: Multiboot is not valid!", COLOR(VIDRed, VIDWhite));
			for(;;);
		}
		DEBUGC(" VALID \n", COLOR(VIDGreen, VIDWhite));
		
		DEBUG("> Installing GDT - ");
		CPU::GDT::gdt_init();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));
		
		DEBUG("> Installing IDT - ");
		CPU::IDT::idt_init();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

		DEBUG("> Installing ISRs - ");
		CPU::ISR::isrs_install();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

		DEBUG("> Installing IRQs (PIC) - ");
		
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

		int j = 0,x = 1;
		term.putc(x/j, 0);
		 
		for(;;);

		return 1;
	}

	void kexit() {
		term.fill(VIDBlue);
		term.puts((char*)"!! The Kernel has exited !!", COLOR(VIDBlue, VIDWhite));
		for(;;) { 
			asm("cli");
			asm("hlt");
		}
	}
}
