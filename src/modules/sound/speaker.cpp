/*
 * speaker.cpp
 *
 *  Created on: 21/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include <fs.h>
#include <modules/sound/speaker.h>

/*******************************************/
/**** Speaker Implementation Functions: ****/
/*******************************************/
static void note(int length, int freq) {
	uint32_t div = 11931800 / freq;
	uint8_t  t;

	/* Configure Timer 2's frequency: */
	outb(0x43, 0xb6);
	outb(0x42, (uint8_t)(div));
	outb(0x42, (uint8_t)(div >> 8));

	/* Start the timer: */
	t = inb(0x61);
	outb(0x61, t | 0x3);

	unsigned long s, ss;
	MOD_IOCTL("pit_driver", 6, 0, (uintptr_t)(length * 10), (uintptr_t)(&s), (uintptr_t)(&ss));
	sleep_until(current_task_get(), s, ss);
	switch_task(0);

	/* Stop the timer: */
	t = inb(0x61) & 0xFC;
	outb(0x61, t);
}

static uint32_t write_speaker(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	if (!size % (sizeof(speaker_t)))
		return 0;

	speaker_t * s = (speaker_t *)buffer;
	while ((uintptr_t)s < (uintptr_t)buffer + size) {
		note(s->length, s->frequency);
		s++;
	}

	return (uintptr_t)s - (uintptr_t)buffer;
}

static FILE * speaker_device_create(void) {
	FILE * fnode = (FILE*)malloc(sizeof(FILE));
	memset(fnode, 0, sizeof(FILE));
	sprintf(fnode->name, "%s", "speaker");
	fnode->flags = FS_CHARDEV;
	fnode->write = write_speaker;
	return fnode;
}
/*******************************************/

/***********************************/
/**** Speaker Module Functions: ****/
/***********************************/
static int speaker_mod_init(void) {
	vfs_mount("/dev/speaker", speaker_device_create());
	return 0;
}

static int speaker_mod_finit(void) {
	return 0;
}

static uintptr_t speaker_ioctl(void * ioctl_packet) {
	return IOCTL_NULL;
}
/***********************************/

MODULE_DEF(mouse_driver, speaker_mod_init, speaker_mod_finit, MODT_AUDIO, "Miguel S.", speaker_ioctl);
