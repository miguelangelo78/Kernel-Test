#include <system.h>
#include <module.h>
#include <stdint.h>

namespace Kernel {
	Terminal term;

	int kmain(struct KInit::multiboot_t * mboot, unsigned magic, uint32_t initial_stack) 
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
	
		DEBUG("\n\nTODO: \n1 - Code up Kernel Heap\n");
		DEBUG("3 - Code up modules\n");
		DEBUG("4 - Relocate Modules\n");
		DEBUG("5 - Enable Paging\n");
		DEBUG("6 - Set up heap pointer\n");
		DEBUG("7 - Enough for now...\n");
		
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
