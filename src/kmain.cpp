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

	void relocate_heap(void) {
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
			kprintf("Moving heap to 0x%x! ", last_mod);
			kheap_starts(last_mod); /* Set new heap pointer */
		}
	}

	/* The linker seems to not put 0's on the symbol table. This confuses the kernel when looking up symbols.
	 * We might want to force the linker to fill the area with 0's... */
	void symboltable_cleanup(void) {
		#define MATCH_OCCURRENCE 0x90669066
		int occurrences = 0;
		uintptr_t * first_occurrence = 0;

		for(uintptr_t * table_addr = (uintptr_t *)KERNEL_SYMBOLS_TABLE_START; table_addr < (uintptr_t *)KERNEL_SYMBOLS_TABLE_END; table_addr++) {
			if(*table_addr == MATCH_OCCURRENCE) {
				if(!occurrences) first_occurrence = table_addr;
				occurrences++;
			} else {
				if(occurrences) { /* Oops, that was actual data. Just a coincidence */
					first_occurrence = 0;
					occurrences = 0;
				}
			}
			if(occurrences >= 3) { /* Matches were exhausted. We'll consider this section to be dirty. */
				for(uintptr_t * table_addr = first_occurrence; table_addr < (uintptr_t *)KERNEL_SYMBOLS_TABLE_END; table_addr++) {
					if(*table_addr != MATCH_OCCURRENCE) break; /* We've reached the end or some data */
					*table_addr = 0x00000000; /* Clean it up */
				}
				break;
			}
		}
	}

	int kmain(struct multiboot_t * mboot, unsigned magic, uint32_t initial_esp)
	{
		/******* Initialize everything: *******/
		/* Initialize critical data: */
		KInit::init_esp = initial_esp;
		mboot_ptr = mboot;
		setup_linker_pointers();
		symboltable_cleanup();

		/* Install the core components very early (because of Serial and command line): */
		CPU::GDT::gdt_init();
		CPU::IDT::idt_init();
		CPU::ISR::isrs_install();
		CPU::IRQ::irq_install();
		relocate_heap();
		paging_install(MEMSIZE());

		CPU::cpuid_t cpuid = CPU::cpuid(CPU::CPUID_GETVENDORSTRING);

		/* Initialize cmdline very early, because the cmdline might contain commands which indicate how to initialize the system */
		if (mboot_ptr->cmdline)	args_parse((char*)mboot_ptr->cmdline);

		int video_mode = args_present("vga") ? atoi(args_value("vga")) : 0;
		video_init(video_mode);
		term.init(video_mode);
		serial.init(COM1);

		Log::redirect_log(args_present("serial") ? Log::LOG_SERIAL : args_present("vga_serial") ? Log::LOG_VGA_SERIAL : Log::LOG_VGA);

		/* Output initial data from multiboot: */
		kprintf("> Bootloader: %s| Bootloader Mod Count: %d at 0x%x\n> Memory: 0x%x ",
			mboot_ptr->boot_loader_name,
			mboot_ptr->mods_count,
			*(uint32_t*)mboot_ptr->mods_addr,
			MEMSIZE());
		kprintfc(COLOR_WARNING, "*%d MB*", MEMSIZE() / 1024);
		kprintf(" (start: 0x%x end: 0x%x = 0x%x)\n", KInit::ld_segs.ld_kstart, KInit::ld_segs.ld_kend, KERNELSIZE());
		kprintf("> CPU: %s (%s)\n> Video: %s (%d) | ESP: 0x%x | Symbols found: %d\n\n",
			CPU::cpu_vendor(), cpu_is_amd(cpuid) ? "AMD" : cpu_is_intel(cpuid) ? "Intel" : "Unknown",
			video_mode_get(video_mode), video_mode, init_esp, symbol_count());
		
		/* Validate Multiboot: */
		kputsc(">> Initializing Kernel <<\n", COLOR_INFO);
		kputs("> Checking Multiboot...");
		ASSERT(magic == MULTIBOOT_HEADER_MAGIC, "Multiboot is not valid!");
		DEBUGVALID();
		
		/* Command line was initialized early: */
		if (mboot_ptr->cmdline) { kputs("> Setting up command line - "); DEBUGOK(); }
		/* GDT was installed early: */
		kputs("> Installing GDT - "); DEBUGOK();
		/* IDT was installed early: */
		kputs("> Installing IDT - "); DEBUGOK();
		/* ISRs were installed early: */
		kputs("> Installing ISRs - "); DEBUGOK();
		/* IRQs were installed early: */
		kputs("> Installing IRQs (PIC) - "); DEBUGOK();
		/* Move heap up because of modules (DONE EARLY): */
		kputs("> Relocating heap - "); DEBUGOK();
		/* Enable paging and heap (DONE EARLY: */
		kputs("> Installing paging and heap - "); DEBUGOK();

		/* Install VFS (with or without initrd): */
		if(mboot_ptr->mods_count > 0) {
			kputs("> Installing VFS (with initrd) - ");
			vfs_install(mboot->mods_addr);
		} else {
			kputs("> Installing VFS - ");
			vfs_install();
		}
		DEBUGOK();

		/* Initialize multitasking: */
		kputs("> Initializing multitasking - "); tasking_install(); DEBUGOK();

		/* Load CORE modules ONLY: */
		kputs("> Loading up modules - "); Module::modules_load(); DEBUGOK();

		/* TODO List: */
		kputsc("\nTODO:\n", COLOR_WARNING);
		kputs("1 - Set up:\n  1.1 - FPU\n  1.2 - Syscalls\n  1.3 - Shared memory");

		/* All done! */
		Log::redirect_log(Log::LOG_VGA_SERIAL);

		kputsc("\nReady", COLOR_GOOD);

		for(;;)
			if(serial.is_ready()) {
				char c = serial.read_async();
				/* Echo back: */
				kprintf("%c", c);
			}
		return 0;
	}

	void kexit()
	{
		Error::infinite_idle("!! The Kernel has exited !!");
	}
}
