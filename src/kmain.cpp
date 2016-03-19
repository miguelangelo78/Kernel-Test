#include <system.h>
#include <libc.h>
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

		/* Initial stack pointer: */
		uintptr_t init_esp = 0;

		/* Linker segments: */
		struct ld_seg ld_segs;

		void setup_linker_pointers(void) {
			ld_segs.ld_kstart = &kstart;
			ld_segs.ld_code = &code;
			ld_segs.ld_end = &end;
			ld_segs.ld_kend = &end;
			ld_segs.ld_data = &data;
			ld_segs.ld_bss = &bss;
			ld_segs.ld_rodata = &rodata;
		}
	}

	/* Terminal which uses text-mode video */
	Terminal term;
	/* Serial port (COM1) which will be used for logging: */
	Serial serial;
	

	void relocate_stack(void) {
		if ((IS_BIT_SET(mboot_ptr->flags, 3)) && mboot_ptr->mods_count > 0) {
			uintptr_t last_mod = (uintptr_t)&KInit::ld_segs.ld_end;

			mboot_mods = (mboot_mod_t*)mboot_ptr->mods_addr;

			/* Iterate through all mods: */
			for (uint32_t i = 0; i < mboot_ptr->mods_count; ++i) {
				mboot_mod_t * mod = &mboot_mods[i];
				if ((uintptr_t)mod + sizeof(mboot_mod_t) > last_mod)
					last_mod = (uintptr_t)mod + sizeof(mboot_mod_t);
				/* Move heap pointer up: */
				if(last_mod < mod->mod_end)
					last_mod = mod->mod_end;
			}
			kheap_starts(last_mod); /* Set new heap pointer */
		}
	}

	int kmain(struct multiboot_t * mboot, unsigned magic, uint32_t initial_esp)
	{
		/******* Initialize everything: *******/
		/* Initialize critical data: */
		KInit::init_esp = initial_esp;
		mboot_ptr = mboot;
		setup_linker_pointers();

		/* Install the core components very early (because of Serial): */
		CPU::GDT::gdt_init();
		CPU::IDT::idt_init();
		CPU::ISR::isrs_install();
		CPU::IRQ::irq_install();

		term.init();
		serial.init(COM1);

		Log::redirect_log(LOG_SERIAL);

		/* Output initial data from multiboot: */
		kprintf("> Bootloader: %s| Module Count: %d at 0x%x\n> Memory: 0x%x ",
			mboot_ptr->boot_loader_name,
			mboot_ptr->mods_count,
			mboot_ptr->mods_addr,
			MEMSIZE());
		kprintfc(COLOR_WARNING, "* %d MB *", MEMSIZE()/1024);
		kprintf(" (start: 0x%x end: 0x%x = 0x%x)\n", KInit::ld_segs.ld_kstart, KInit::ld_segs.ld_kend, KERNELSIZE());
		kprintf("> ESP: 0x%x\n\n", init_esp);
		
		/* Validate Multiboot: */
		kputsc(">> Initializing Kernel <<\n", COLOR_INFO);
		kputs("> Checking Multiboot...");
		ASSERT(magic == MULTIBOOT_HEADER_MAGIC, "Multiboot is not valid!");
		DEBUGVALID();
		
		/* GDT was installed early: */
		kputs("> Installing GDT - "); DEBUGOK();
		/* IDT was installed early: */
		kputs("> Installing IDT - "); DEBUGOK();
		/* ISRs were installed early: */
		kputs("> Installing ISRs - "); DEBUGOK();
		/* IRQs were installed early: */
		kputs("> Installing IRQs (PIC) - "); DEBUGOK();
		
		/* Move stack up because of modules: */
		kputs("> Relocating stack - ");
		relocate_stack();
		DEBUGOK();

		/* Enable paging and heap: */
		kputs("> Installing paging and heap - ");
		paging_install(MEMSIZE());
		DEBUGOK();

		kputs("> Installing VFS - ");
		vfs_install();
		DEBUGOK();

		kputs("> Setting up command line - ");
		if (mboot_ptr->cmdline) args_parse((char*)mboot_ptr->cmdline);
		DEBUGOK();

		/* TODO List: */
		kputsc("\nTODO:\n", COLOR_WARNING);
		kputs("1 - Set up: \n  1.1 - Tasking"
			"\n  1.2 - Timer\n  1.3 - FPU"
			"\n  1.4 - Syscalls\n  1.5 - Shared memory"
			"\n  1.6 - Init modules\n")
		kputs("2 - Code up modules and load them\n");

		/* All done! */
		kputsc("\nReady", COLOR_GOOD);

		Log::redirect_log(LOG_VGA);
		kputsc("Ready", COLOR_GOOD);

		for(;;)
			if(serial.is_ready()) {
				char c = serial.read_async();
				/* Echo back: */
				kprintf("%c", c);
				serial.write(c);
			}
		return 0;
	}

	void kexit()
	{
		Error::infinite_idle("!! The Kernel has exited !!");
	}
}
