/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <kernel_headers/kheaders.h>
#include <system.h>

static int keyboard_handler(Kernel::CPU::regs_t * regs) {
	kprintf("%c\n", Kernel::inb(0x60));
	return 0;
}

static int keyboard_ini(void) {
	symbol_call_args("irq_install_handler", 2, 1, keyboard_handler);
	return 0;
}

static int keyboard_fini(void) {
	return 0;
}

MODULE_DEF(keyboard_driver, keyboard_ini, keyboard_fini);

