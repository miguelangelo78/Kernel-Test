#include "console.h"
#include "libc.h"

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

void * memset(void * dest, int c, size_t n) {
	uint8_t *ptr = (uint8_t*)dest;
	for(size_t i = 0; i < n; i++)
		ptr[i] = (uint8_t)c;
	return dest;
}

int strcmp(const char * l, const char * r) {
	for (; *l == *r && *l; l++, r++);
	return *(uint8_t *)l - *(uint8_t *)r;
}

size_t strlen(const char * s) {
	const char * a = s;
	const size_t * w;
	for (; (uintptr_t)s % ALIGN; s++) {
		if (!*s) {
			return s - a;
		}
	}
	for (w = (const size_t *)s; !HASZERO(*w); w++);
	for (s = (const char *)w; *s; s++);
	return s - a;
}

namespace Module {
	extern "C" { void kernel_symbols_start(void); }
	extern "C" { void kernel_symbols_end(void); }

	typedef struct {
		uintptr_t addr;
		char name[];
	} kernel_sym_t;
	
	void * symbol_find(const char * name) {
		kernel_sym_t * k = (kernel_sym_t *)&kernel_symbols_start;
		while ((uintptr_t)k < (uintptr_t)&kernel_symbols_end) {
			if (strcmp(k->name, name)) {
				k = (kernel_sym_t *)((uintptr_t)k + sizeof *k + strlen(k->name) + 1);
				continue;
			}
			return (void*)k->addr;
		}
		return NULL;
	}

	void * symbol_call(const char * name, void * params) {
		typedef void * (*cback)(void*);
		cback fptr = (cback)Module::symbol_find(name);
		if(fptr) return fptr(params);
		else return NULL;
	}

	void * symbol_call(const char * name) {
		typedef void * (*cback)(void);
		cback fptr = (cback)Module::symbol_find(name);
		if (fptr) return fptr();
		else return NULL;
	}
}

namespace Kernel {
	#define asm __asm__
	#define volatile __volatile__

	#define DEBUG(msg) term.puts((char*)msg, DEFAULT_COLOR);
	#define DEBUGC(msg, color) term.puts((char*)msg, color);

	#define KERNEL_STOP() for(;;) { asm("cli"); asm("hlt"); }

	#define SYSCALL_VECTOR 0x7F

	Console term; /* Used only for debugging to the screen */

	namespace Error {
		void panic(const char * msg, int intno) {
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
	
		namespace ISR { /* ISR: Uses the IDT to install and manage exceptions */
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
			
			static irq_handler_t isr_routines[256];

			void isr_install_handler(size_t isrs, irq_handler_t handler) {
				isr_routines[isrs] = handler;
			}

			void isr_uninstall_handler(size_t isrs) {
				isr_routines[isrs] = 0;
			}

			void isrs_install(void) {
				char buffer[16];
				for (int i = 0; i < ISR_COUNT; i++) {
					sprintf(buffer, "_isr%d", i);
					IDT::idt_set_gate(i, (IDT::idt_gate_t)Module::symbol_find(buffer), 0x08, 0x8E);
				}
				
				IDT::idt_set_gate(SYSCALL_VECTOR, (IDT::idt_gate_t)Module::symbol_find("_isr127"), 0x08, 0x8E);
			}
			
			void fault_handler(CPU::regs_t * r) {
				irq_handler_t handler = isr_routines[r->int_no];
				if (handler)
					handler(r);
				else /* Kernel BSOD */
					Error::panic(exception_msgs[r->int_no], r->int_no);
			}
		}

		namespace IRQ { /* IRQ: Uses the IDT to install and manage interrupt requests */

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

		asm("int $1");

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
