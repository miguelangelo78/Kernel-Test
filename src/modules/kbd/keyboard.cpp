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
#include "../fs/pipe.h"

#define KEY_DEVICE  0x60
#define KEY_PENDING 0x64

#define KBD_PIPE_DEPTH 128

static FILE * keyboard_pipe;

static void keyboard_wait(void) {
	while(Kernel::inb(KEY_PENDING) & 2);
}

static int keyboard_handler(Kernel::CPU::regs_t * regs) {
	keyboard_wait();
	char scan = Kernel::inb(KEY_DEVICE);
	if(!(scan & 0x80)) {
		if(scan==72)
			scup(); // XXX: REMOVE LATER
		else if(scan==80)
			scwn(); // XXX: REMOVE LATER
		else if(scan==75)
			sctop(); // XXX: REMOVE LATER
		else if(scan==77)
			scbot(); // XXX: REMOVE LATER
		else {
			uint8_t b[1];
			b[0] = kbdus[scan];
			fwrite(keyboard_pipe, 0, 1, b);
		}
	}
	return 0;
}

static int keyboard_sched_ini(void) {
	/* Prepare and mount keyboard onto the filesystem: */
	keyboard_pipe = make_pipe(KBD_PIPE_DEPTH);
	keyboard_pipe->flags = FS_CHARDEV;
	vfs_mount("/dev/kbd", keyboard_pipe);
	/* Install interrupt handler: */
	SYA(irq_install_handler, Kernel::CPU::IRQ::IRQ_KBD, keyboard_handler);
	return 0;
}

static int keyboard_ini(void) {
	/* We are only allowed to use the keyboard once we have the pipe working: */
	module_schedule_quick("pipe_driver", keyboard_sched_ini);
	return 0;
}

static int keyboard_fini(void) {
	return 0;
}

static uintptr_t keyboard_ioctl(void * ioctl_packet) {
	SWITCH_IOCTL(ioctl_packet) {
		case 0: return (uintptr_t)keyboard_pipe;
	}
	return 0;
}

MODULE_DEF(keyboard_driver, keyboard_ini, keyboard_fini, MODT_PS2, "Miguel S.", keyboard_ioctl);

