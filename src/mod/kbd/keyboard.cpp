/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <kernel_headers/kheaders.h>
#include <system.h>
#include "kbd_scan.h"

static void keyboard_wait(void) {
	while(Kernel::inb(0x64) & 2);
}

static int keyboard_handler(Kernel::CPU::regs_t * regs) {
	keyboard_wait();
	char scan = Kernel::inb(0x60);
	if(!(scan & 0x80))
		kprintf("%c", kbdus[scan]);
	return 0;
}

static int keyboard_ini(void) {
	SYA(irq_install_handler, Kernel::CPU::IRQ::IRQ_KBD, keyboard_handler);
	return 0;
}

static int keyboard_fini(void) {
	return 0;
}

MODULE_DEF(keyboard_driver, keyboard_ini, keyboard_fini, MODT_PS2, "Miguel S.");

