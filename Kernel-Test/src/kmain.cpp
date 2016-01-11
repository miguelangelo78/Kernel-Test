// $LLVMDISABLE(1)
#include <system.h>
#include <module.h>
#include <stdint.h>
#include <log.h>
#include <args.h>
#include <fs.h>

namespace Kernel {
	namespace KInit {
		/* Multiboot pointer: */
		struct multiboot_t * mboot_ptr = 0;
		mboot_mod_t * mboot_mods = (mboot_mod_t*)0;
	}

	/* Terminal which uses text-mode video */
	Terminal term;

	/* Initial stack pointer: */
	uintptr_t init_esp = 0;
	
	/* Linker segments: */
	struct ld_seg ld_segs;

	void relocate_stack(void) {
		if (IS_BIT_SET(mboot_ptr->flags, 3) && mboot_ptr->mods_count > 0) {
			uintptr_t last_mod = (uintptr_t)&ld_segs.ld_end;

			mboot_mods = (mboot_mod_t*)mboot_ptr->mods_addr;

			for (uint32_t i = 0; i < mboot_ptr->mods_count; ++i) {
				mboot_mod_t * mod = &mboot_mods[i];
				if ((uintptr_t)mod + sizeof(mboot_mod_t) > last_mod)
					last_mod = (uintptr_t)mod + sizeof(mboot_mod_t);
				/* Move heap pointer up: */
				if(last_mod < mod->mod_end)
					last_mod = mod->mod_end;
			}
			kmalloc_starts(last_mod); /* Set new heap pointer */
		}
	}

	void setup_cmdline(void* cmdline) { 
		if (cmdline) 
			args_parse((char*)cmdline);
	}

	void setup_linker_pointers(void) {
		ld_segs.ld_code = &code;
		ld_segs.ld_end = &end;
		ld_segs.ld_data = &data;
		ld_segs.ld_bss = &bss;
		ld_segs.ld_rodata = &rodata;
	}

	int kmain(struct multiboot_t * mboot, unsigned magic, uint32_t initial_stack) 
	{
		/******* Initialize everything: *******/
		term.init();
		
		/* Initialize critical data: */
		init_esp = initial_stack;
		mboot_ptr = mboot;
		setup_linker_pointers();

		/* Output initial data from multiboot: */
		DEBUGF("> Bootloader: %s| Module Count: %d at 0x%x\n> Memory: 0x%x\n", 
			mboot_ptr->boot_loader_name,
			mboot_ptr->mods_count,
			mboot_ptr->mods_addr,
			MEMSIZE());
		DEBUGF("> ESP: 0x%x\n\n", init_esp);
		
		/* Validate Multiboot: */
		DEBUGC(">> Initializing Kernel <<\n", COLOR_INFO);
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
		
		/* Move stack up because of modules: */
		DEBUG("> Relocating stack - ");
		relocate_stack();
		DEBUGOK();

		/* Enable paging and heap: */
		DEBUG("> Installing paging and heap - ");
		paging_enable(MEMSIZE());
		heap_install();
		DEBUGOK();

		DEBUG("> Setting up command line - ");
		setup_cmdline((void*)mboot_ptr->cmdline);
		DEBUGOK();

		DEBUG("> Installing VFS - ");
		vfs_install();
		DEBUGOK();

		/* TODO List: */
		DEBUGC("\nTODO:\n", COLOR_WARNING);
		DEBUG("1 - Set up: \n  1.1 - Tasking"
			"\n  1.2 - Timer\n  1.3 - FPU"
			"\n  1.4 - Syscalls\n  1.5 - Shared memory"
			"\n  1.6 - Init modules\n")
		DEBUG("2 - Code up modules and load them\n");

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
