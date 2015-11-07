#include "console.h"
#include "stdint.h"

void * memset(void * dest, int c, size_t n) {
	unsigned char *ptr = (unsigned char*)dest;
	for(size_t i = 0 ; i < n; i++)
		ptr[i] = c;
	return dest;
}

namespace Kernel {
	#define asm __asm__
	#define volatile __volatile__
	#define attribute(attr) __attribute__((attr))

	#define DEBUG(msg) term.puts((char*)msg, DEFAULT_COLOR);
	#define DEBUGC(msg, color) term.puts((char*)msg, color);

	Console term; /* Used only for debugging to the screen */

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
			void gdt_init(void) {
				
			}
		}

		namespace IDT {
			#define IDTENTRY(entry) idt.entries[(entry)]

			typedef void(*idt_gate_t)(void);

			typedef struct {
				uint16_t base_low;
				uint16_t sel;
				uint8_t zero;
				uint8_t flags;
				uint16_t base_high;
			} attribute(packed) idt_entry_t;

			typedef struct {
				uint16_t limit;
				uintptr_t base;
			} attribute(packed) idt_ptr_t;

			static struct {
				idt_entry_t entries[256];
				idt_ptr_t ptr;
			} idt attribute(used);

			void idt_set_gate(uint8_t num, idt_gate_t base, uint16_t sel, uint8_t flags) {
				IDTENTRY(num).base_low = ((uintptr_t)base & 0xFFFF);
				IDTENTRY(num).base_high = ((uintptr_t)base >> 16) & 0xFFFF;
				IDTENTRY(num).sel = sel;
				IDTENTRY(num).zero = 0;
				IDTENTRY(num).flags = flags | 0x60;
			}

			void idt_flush() {
				asm volatile("mov 4(%esp), %eax; lidt (%eax);");
			}

			void init() {
				/* Set up IDT pointer: */
				idt_ptr_t * idt_ptr = &idt.ptr;
				idt_ptr->limit = sizeof(idt.entries - 1);
				idt_ptr->base = (uintptr_t)&IDTENTRY(0);
				memset(&IDTENTRY(0), 0 ,sizeof(idt.entries));

				/* Install IDT: */
				idt_flush();
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
			static const char *exception_messages[ISR_COUNT] = {
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
			} isrs[32 + 1] attribute(used);

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

			void fault_handler(CPU::regs_t * r) {
				for(;;);
				irq_handler_t handler = isr_routines[r->int_no];
				if (handler) {
					handler(r);
				} else {
					// Kernel BSOD
					char * vid = (char*)0xB8000;
					vid[0]='a';
				
					for(;;);
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
		typedef struct 
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
		} multiboot_t attribute(packed);
	}

	int kmain(Init::multiboot_t * mboot, unsigned magic, uint32_t initial_stack) 
	{
		/* Initialize everything: */
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
		CPU::IDT::init();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

		DEBUG("> Installing ISRs - ");
		CPU::ISR::isrs_install();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

		DEBUG("> Installing IRQs (PIC) - ");
		
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

		for (;;);

		int j = 0,x = 1;
		term.putc(x/j, 0);
		 
		for(;;);
	}
}
