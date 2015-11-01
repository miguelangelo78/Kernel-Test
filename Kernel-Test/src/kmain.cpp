#include "console.h"
#include "stdint.h"

#define asm __asm__
#define volatile __volatile__
#define attribute(attr) __attribute__((attr))

void * memset(void * dest, int c, size_t n) {
	unsigned char *ptr = (unsigned char*)dest;
	for(size_t i = 0 ; i < n; i++)
		ptr[i] = c;
	return dest;
}

namespace HAL {
	namespace IDT {
		#define IDTENTRY(entry) idt.entries[(entry)]

		/* ISR */
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

		typedef void (*idt_gate_t)(void);

		void idt_set_gate(uint8_t num, idt_gate_t base, uint16_t sel, uint8_t flags) {
			IDTENTRY(num).base_low = ((uintptr_t)base & 0xFFFF);
			IDTENTRY(num).base_high = ((uintptr_t)base >> 16) & 0xFFFF;
			IDTENTRY(num).sel = sel;
			IDTENTRY(num).zero = 0;
			IDTENTRY(num).flags = flags | 0x60;
		}

		void init() {
			/* Set up IDT pointer: */
			idt_ptr_t * idt_ptr = &idt.ptr;
			idt_ptr->limit = sizeof(idt.entries - 1);
			idt_ptr->base = (uintptr_t)&IDTENTRY(0);
			memset(&IDTENTRY(0), 0 ,sizeof(idt.entries));

			/* Install IDT: */
			asm volatile("mov 4(%esp), %eax; lidt (%eax);");
		}
	}
	
	namespace ISR {
		#define ISR_COUNT 32

		typedef void(*irq_handler_t) (struct regs *);

		static struct {
			size_t index;
			void (*stub)(void);
		} isrs[32 + 1] attribute(used);

		static irq_handler_t isr_routines[256];

		static const char *exception_messages[32] = {
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

		void isr_install(void) {

		}

		/*void fault_handler(struct regs * r) {
			irq_handler_t handler = isr_routines[r->int_no];
			if (handler) {
				handler(r);
			} else {
				// Kernel BSOD
			}
		}*/
	}
	
	namespace IRQ {

	}
}

namespace Kernel {
	#define DEBUG(msg) term.puts((char*)msg, DEFAULT_COLOR);
	#define DEBUGC(msg, color) term.puts((char*)msg, color);

	void kinit() {
		Console term;
		DEBUG(">> Initializing Kernel <<\n\n");

		/* These features were already initialized on the stage 2 bootloader: */
		DEBUG("> Checking Multiboot - "); DEBUGC(" VALID \n", COLOR(VIDGreen, VIDWhite));
		DEBUG("> Installing GDT - "); DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));
		DEBUG("> Switching to Protected Mode (32 bits) - "); DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));
		DEBUG("> Moving Stack Pointer up - "); DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));
		DEBUG("> Setting up Paging and jumping to Higher Half kernel - "); DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));
		
		/* From now on, the initializations will happen here instead:  */
		DEBUG("> Installing IDT - "); 
		HAL::IDT::init();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));
		
		DEBUG("> Setting up ISRs - "); 
		HAL::ISR::isr_install();
		DEBUGC(" OK \n", COLOR(VIDGreen, VIDWhite));

	}
}

void kmain() {
	Kernel::kinit();
	for(;;);
}
