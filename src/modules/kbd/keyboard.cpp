/*
 * mod.c
 *
 *  Created on: 21/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include "kbd_scan.h"

#define KEY_DEVICE  0x60
#define KEY_PENDING 0x64

static FILE * keyboard_pipe;

static void keyboard_wait(void) {
	while(Kernel::inb(KEY_PENDING) & 2);
}

static int keyboard_handler(Kernel::CPU::regs_t * regs) {
	keyboard_wait();
	char scan = Kernel::inb(KEY_DEVICE);
	if(!(scan & 0x80)) {
		if(scan==72)
			scup();
		else if(scan==80)
			scwn();
		else if(scan==75)
			sctop();
		else if(scan==77)
			scbot();
		else {
			uint8_t b[1];
			b[0] = kbdus[scan];
			fwrite(keyboard_pipe, 0, 1, b);
			kprintf("%c", kbdus[scan]); // XXX REMOVE LATER
		}
	}
	keyboard_wait();
	return 0;
}

static int keyboard_ini(void) {
	/* TODO TODO TODO TODO: Needs pipe! */
	/* Prepare and mount keyboard onto the filesystem: */
	//keyboard_pipe = (FILE*)malloc(sizeof(FILE));
	//memset(keyboard_pipe, 0, sizeof(FILE));

	//keyboard_pipe->flags = FS_CHARDEV;
	vfs_mount("/dev/kbd", keyboard_pipe);
	/* Interrupt handler: */
	SYA(irq_install_handler, Kernel::CPU::IRQ::IRQ_KBD, keyboard_handler);
	return 0;
}

static int keyboard_fini(void) {
	return 0;
}

MODULE_DEF(keyboard_driver, keyboard_ini, keyboard_fini, MODT_PS2, "Miguel S.");

