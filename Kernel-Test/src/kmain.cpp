#include <system.h>
#include <module.h>
#include <stdint.h>
#include <log.h>

namespace Kernel {
	Terminal term;

	/* Initial stack pointer: */
	uintptr_t init_esp = 0;
	
	/* Linker segments: */
	struct ld_seg ld_segs;

	int kmain(struct multiboot_t * mboot, unsigned magic, uint32_t initial_stack) 
	{
		/******* Initialize everything: *******/
		term.init();

		/* Initialize critical data: */
		init_esp = initial_stack;
		mboot_ptr = mboot;
		ld_segs = { &code, &end, &data, &bss, &rodata };

		/* Output initial data from multiboot: */
		DEBUGF("> Bootloader: %s| Module Count: %d at 0x%x\n> Memory: 0x%x\n", 
			mboot_ptr->boot_loader_name, 
			mboot_ptr->mods_count,
			mboot_ptr->mods_addr,
			mboot_ptr->mem_upper - KInit::mboot_ptr->mem_lower);
		DEBUGF("> ESP: 0x%x\n\n", init_esp);

		/* Validate Multiboot: */
		DEBUGC(">> Initializing Kernel <<\n\n", COLOR_INFO);
		DEBUG("> Checking Multiboot...");
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
	
		/* TODO List: */
		DEBUGC("\n\nTODO:\n", COLOR_WARNING);
		DEBUG("1 - Code up Kernel Heap\n");
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
