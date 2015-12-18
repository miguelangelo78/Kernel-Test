#pragma once

#include <kinit.h>
#include <terminal.h>
#include <libc.h>
#include <io.h>
#include <attr.h>
#include <bit.h>
#include <process.h>

/* Memory segment selectors: */
enum SEGSEL {
	SEG_NULL,
	SEG_KERNEL_CS = 0x8,
	SEG_KERNEL_DS = 0x10,
	SEG_USER_CS = 0x18,
	SEG_USER_DS = 0x20
};

/* Segments from the linker */
extern void * code;
extern void * end;
extern void * data;
extern void * bss;
extern void * rodata;
/* Segments from the linker (in struct form) */
struct ld_seg {
	void * ld_code;
	void * ld_end;
	void * ld_data;
	void * ld_bss;
	void * ld_rodata;
};
extern struct ld_seg ld_segs;
extern uintptr_t init_esp;

#define asm __asm__
#define volatile __volatile__

#define ASSERT(cond, msg) { if(!(cond)) { char buff[256]; sprintf(buff, "Assert (%s): %s", STR(cond), msg); Error::panic(buff, __LINE__, __FILE__, 0); } }

#define IRQ_OFF() Kernel::CPU::IRQ::int_disable()
#define IRQ_RES() Kernel::CPU::IRQ::int_resume()
#define IRQ_ON() Kernel::CPU::IRQ::int_enable()

#define KERNEL_PAUSE() { asm volatile ("hlt"); }
#define KERNEL_FULL_PAUSE() while (1) { KERNEL_PAUSE(); }

#define KERNEL_FULL_STOP() while(1) { IRQ_OFF(); KERNEL_FULL_PAUSE(); }

typedef volatile int spin_lock_t[2];
extern void spin_init(spin_lock_t lock);
extern void spin_lock(spin_lock_t lock);
extern void spin_unlock(spin_lock_t lock);

extern void switch_task(uint8_t reschedule);

namespace Kernel {
	extern Terminal term;

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
			void gdt_init(void);
			void gdt_set_gate(uint8_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran);
		}

		namespace IDT {
			/* IDT Interrupt List (includes ISRs and IRQs): */
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

			void idt_init();
			void idt_set_gate(uint8_t num, uintptr_t isr_addr, uint16_t sel, uint8_t flags);
		}

		namespace ISR {
			#define ISR_COUNT 32

			/* ISR Messages: */
			extern const char *exception_msgs[ISR_COUNT];

			/* Function callback type for ISRs: */
			typedef void(*isr_handler_t) (CPU::regs_t *);

			void isrs_install(void);
			void isr_install_handler(size_t isrs, isr_handler_t handler);
			void isr_uninstall_handler(size_t isrs);
		}

		namespace IRQ {
			typedef int(*irq_handler_t) (CPU::regs_t *); /* Callback type */

			void irq_install(void);

			void int_disable(void);
			void int_enable(void);
			void int_resume(void);

			void irq_install_handler(size_t irq_num, irq_handler_t irq_handler);
			void irq_uninstall_handler(size_t irq_num);
			void irq_ack(size_t irq_num);
			void irq_mask(uint8_t irq_num, uint8_t enable);
			void irq_set_mask(uint8_t irq_num);
			void irq_clear_mask(uint8_t irq_num);
		}
	}

	namespace Memory {

		/* Kernel memory manager. Contains paging/physical memory functions and a Kernel memory 
		allocator (kmalloc, a dumb version of Alloc's malloc to be used before paging is enabled) */
		namespace Man {
			void kmalloc_starts(uintptr_t start_addr);
			uintptr_t kmalloc(size_t size, char align, uintptr_t * phys);
			uintptr_t kmalloc(size_t size);
			uintptr_t kvmalloc(size_t size);
			uintptr_t kmalloc_p(size_t size, uintptr_t *phys);
			uintptr_t kvmalloc_p(size_t size, uintptr_t *phys);

			void paging_install(uint32_t memsize);
			void heap_install(void);
		}

		/* Proper memory allocator to be used after paging and heap are fully installed: */
		namespace Alloc {

		}
	}

	/* Error related functions: */
	namespace Error{
		void panic(const char * msg, const int line, const char * file, int intno);
		void panic(const char * msg, int intno);
		void panic(const char * msg);
		void panic(void);
		void infinite_idle(const char * msg);
	}
}

using namespace Kernel::Memory::Man;
using namespace Kernel::Memory::Alloc; 
using namespace Kernel::Error;
using namespace Kernel::KInit;
