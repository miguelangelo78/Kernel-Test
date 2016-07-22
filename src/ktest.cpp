/*
 * ktest.cpp
 *
 *  Created on: 22/07/2016
 *      Author: Miguel
 */

#include <libc.h>
#include <module.h>
#include <system.h>
#include <stdint.h>
#include <log.h>
#include <args.h>
#include <fs.h>
#include <version/version.h>
#include <time.h>
#include <elf.h>
#include <modules/mouse/mouse.h>
#include <modules/sound/speaker.h>

namespace Kernel {

void test_kernel(void) {
	/***************************************************/
	/***************************************************/
	/************ TEST EVERYTHING DOWN HERE ************/
	/***************************************************/
	/***************************************************/

	/*********** Test drivers: ***********/
	FILE * kbd_file = kopen("/dev/kbd", O_RDONLY);
	uint8_t * kbd_buff;
	if(kbd_file) kbd_buff = (uint8_t*)malloc(128);

	FILE * mouse_file = kopen("/dev/mouse", O_RDONLY);
	uint8_t * mouse_buff;
	if(mouse_file) mouse_buff = (uint8_t*)malloc(sizeof(mouse_device_packet_t) * MOUSE_PACKETS_IN_PIPE);
	int x = 0, y = 0;

	FILE * spkr_file   = kopen("/dev/speaker", O_RDONLY);
	if(spkr_file) {
		speaker_t d;
		d.frequency = 500;
		d.length = 30;
		fwrite(spkr_file, 0, sizeof(speaker_t), (uint8_t*)&d);
	}

	FILE * cmos_file   = kopen("/dev/cmos",    O_RDONLY);
	FILE * pit_file    = kopen("/dev/timer",   O_RDONLY);
	FILE * serial_file = kopen("/dev/com1",    O_RDONLY);
	uint8_t * serial_buff;
	if(serial_file) serial_buff = (uint8_t*)malloc(SERIAL_CBACK_BUFFER_SIZE);

	for(;;) {
		/* Echo serial comm back: */
		if(serial.is_ready() && serial_file) {
#if 0
			/* The normal way to access the serial COM: */
			kprintf("%c", serial.read_async());
#else
			/* Using the VFS: */
			int size;
			MOD_IOCTLD("pipe_driver", size, 1, (uintptr_t)serial_file);
			if(size) {
				fread(serial_file, 0, size, serial_buff);
				kprintf("%c", serial_buff[0]);
				serial.flush();
			}
#endif
		}

		/* Echo keyboard back: */
		if(kbd_file) {
			int size;
			MOD_IOCTLD("pipe_driver", size, 1, (uintptr_t)kbd_file);
			if(size) {
				fread(kbd_file, 0, size, kbd_buff);
				kprintf("%c", kbd_buff[0]);

				if(kbd_buff[0] == 'r') {
					/*********** Test ELF exec: ***********/
					system("/runme.o", 0, 0, 0x123456); /* The entry point was chosen randomly for testing purposes */
				}

				if(kbd_buff[0] == 'p' && spkr_file) {
					/*********** Test PC Speaker: ***********/
					speaker_t d;
					d.frequency = 1000;
					d.length = 100;
					fwrite(spkr_file, 0, sizeof(speaker_t), (uint8_t*)&d);
				}
			}
		}

		/* Show Mouse data: */
		if(mouse_file) {
			int size;
			MOD_IOCTLD("pipe_driver", size, 1, (uintptr_t)mouse_file);
			if(size) {
				fread(mouse_file, 0, size, mouse_buff);
				mouse_device_packet_t * d = (mouse_device_packet_t*)mouse_buff;
				x += d->x_difference;
				if(x>80) x=80;
				if(x<0)  x=0;
				y += d->y_difference;
				if(y>86) y=86;
				if(y<0)  y=0;
				term.printf_at(55, 22, "Mouse - x: %d y: %d         ", x, 86-y);
			}
		}

		/* Show now(): */
		if(cmos_file) {
			IRQ_OFF();
			term.printf_at(65, 24, "Now: %d", fs_ioctl(cmos_file, 4, 0));
			IRQ_RES();
		}

		/* Show pit's ticks and subticks: */
		if(pit_file) {
			IRQ_OFF();
			term.printf_at(55, 23, "Ticks: %d Subticks: %d", fs_ioctl(pit_file, 3, 0), fs_ioctl(pit_file, 4, 0));
			IRQ_RES();
		}
	}
}

}
