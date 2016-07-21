/*
 * mouse.cpp
 *
 *  Created on: 21/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include <stdint.h>
#include <fs.h>
#include <modules/fs/pipe.h>
#include "mouse.h"

/********************/
/**** Variables: ****/
/********************/
static uint8_t mouse_cycle = 0;
static int8_t  mouse_byte[4];
static int8_t  mouse_mode = MOUSE_DEFAULT;
static FILE *  mouse_pipe;

static int mouse_ioctl(FILE * node, int request, void * argp); /* Prototype */
/********************/

/*****************************************/
/**** Mouse Implementation Functions: ****/
/*****************************************/
static void mouse_wait(uint8_t a_type) {
	uint32_t timeout = 100000;
	if (!a_type) {
		while (--timeout)
			if ((inb(MOUSE_STATUS) & MOUSE_BBIT) == 1)
				return;
		return;
	} else {
		while (--timeout)
			if (!((inb(MOUSE_STATUS) & MOUSE_ABIT)))
				return;
		return;
	}
}

static void mouse_write(uint8_t data) {
	mouse_wait(1);
	outb(MOUSE_STATUS, MOUSE_WRITE);
	mouse_wait(1);
	outb(MOUSE_PORT, data);
}

static uint8_t mouse_read(void) {
	mouse_wait(0);
	return inb(MOUSE_PORT);
}

static int mouse_handler(Kernel::CPU::regs_t * r) {
	uint8_t status = inb(MOUSE_STATUS);
	while (status & MOUSE_BBIT) {
		int8_t mouse_in = inb(MOUSE_PORT);
		if (status & MOUSE_F_BIT) {
			switch (mouse_cycle) {
				case 0:
					mouse_byte[0] = mouse_in;
					if (!(mouse_in & MOUSE_V_BIT)) { goto read_next; }
					++mouse_cycle;
					break;
				case 1:
					mouse_byte[1] = mouse_in;
					++mouse_cycle;
					break;
				case 2:
					mouse_byte[2] = mouse_in;
					if (mouse_mode == MOUSE_SCROLLWHEEL || mouse_mode == MOUSE_BUTTONS) {
						++mouse_cycle;
						break;
					}
					goto finish_packet;
				case 3:
					mouse_byte[3] = mouse_in;
					goto finish_packet;
			}
			goto read_next;
finish_packet:
			mouse_cycle = 0;
			if ((mouse_byte[0] & 0x80) || (mouse_byte[0] & 0x40)) {
				/* x/y overflow? bad packet! */
				goto read_next;
			}
			/* We now have a full mouse packet ready to use */
			mouse_device_packet_t packet;
			packet.magic = MOUSE_MAGIC;
			packet.x_difference = mouse_byte[1];
			packet.y_difference = mouse_byte[2];
			packet.buttons = (mouse_click_t)0;
			if (mouse_byte[0] & 0x01)
				packet.buttons = (mouse_click_t)((uintptr_t)packet.buttons | LEFT_CLICK);
			if (mouse_byte[0] & 0x02)
				packet.buttons = (mouse_click_t)((uintptr_t)packet.buttons | RIGHT_CLICK);
			if (mouse_byte[0] & 0x04)
				packet.buttons = (mouse_click_t)((uintptr_t)packet.buttons | MIDDLE_CLICK);

			if (mouse_mode == MOUSE_SCROLLWHEEL && mouse_byte[3]) {
				if (mouse_byte[3] > 0)
					packet.buttons = (mouse_click_t)((uintptr_t)packet.buttons | MOUSE_SCROLL_DOWN);
				else if (mouse_byte[3] < 0)
					packet.buttons = (mouse_click_t)((uintptr_t)packet.buttons | MOUSE_SCROLL_UP);
			}

			mouse_device_packet_t bitbucket;
			while (pipe_size(mouse_pipe) > (int)(MOUSE_DISCARD_POINT * sizeof(packet)))
				fread(mouse_pipe, 0, sizeof(packet), (uint8_t *)&bitbucket);
			fwrite(mouse_pipe, 0, sizeof(packet), (uint8_t *)&packet);
		}
read_next:
		status = inb(MOUSE_STATUS);
	}

	irq_ack(MOUSE_IRQ);
	return 1;
}
/*****************************************/


/*********************************/
/**** Mouse Module Functions: ****/
/*********************************/
static int mouse_mod_init_sched(void) {
	uint8_t status, result;
	IRQ_OFF();

	/* Create Mouse Pipe: */
	mouse_pipe = make_pipe(sizeof(mouse_device_packet_t) * MOUSE_PACKETS_IN_PIPE);

	/* Initialize Mouse: */
	mouse_wait(1);
	outb(MOUSE_STATUS, 0xA8);
	mouse_wait(1);
	outb(MOUSE_STATUS, 0x20);
	mouse_wait(0);
	status = inb(0x60) | 2;
	mouse_wait(1);
	outb(MOUSE_STATUS, 0x60);
	mouse_wait(1);
	outb(MOUSE_PORT, status);
	mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();

	/* Enable Mouse scroll: */
	mouse_write(0xF2);
	mouse_read();
	result = mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(100);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(80);
	mouse_read();
	mouse_write(0xF2);
	mouse_read();
	result = mouse_read();
	if (result == 3)
		mouse_mode = MOUSE_SCROLLWHEEL;

	irq_install_handler(Kernel::CPU::IRQ::IRQ_MOUSE, mouse_handler);
	IRQ_RES();

	uint8_t tmp = inb(0x61);
	outb(0x61, tmp | 0x80);
	outb(0x61, tmp & 0x7F);
	inb(MOUSE_PORT);

	/* Mount the Mouse driver into the VFS: */
	mouse_pipe->flags = FS_CHARDEV;
	mouse_pipe->ioctl = mouse_ioctl;
	vfs_mount("/dev/mouse", mouse_pipe);
	return 0;
}

static int mouse_mod_init(void) {
	/* We are only allowed to use the mouse once we have the pipe working: */
	module_schedule_quick("pipe_driver", mouse_mod_init_sched);
	return 0;
}

static int mouse_mod_finit(void) {
	return 0;
}

static int mouse_ioctl(FILE * node, int request, void * argp) {
	switch(request) {
	case 1: mouse_cycle = 0; return 0;
	}
	return IOCTL_NULL;
}

static uintptr_t mouse_ioctl_mod(void * ioctl_packet) {
	uintptr_t * d = (uintptr_t*)ioctl_packet;
	return mouse_ioctl(0, *d, d + 1);
}
/*********************************/

MODULE_DEF(mouse_driver, mouse_mod_init, mouse_mod_finit, MODT_PS2, "Miguel S.", mouse_ioctl_mod);
