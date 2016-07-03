/*
 * ext2.cpp
 *
 *  Created on: 02/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>

static int ext2_init_sched(void) {

	return 0;
}

static int ext2_init(void) {
	/* Initialize EXT2 after everything else.
	 * This is because the initrd is still being used on the VFS.
	 * If we switched now into ext2 we'd crash and could
	 * not initialize the other modules */
	module_schedule(ext2_init_sched);
	kprintf(" Init scheduled");
	return 1;
}

static int ext2_finit(void) {
	return 0;
}

static uintptr_t ext2_ioctl(void * ioctl_packet) {

	return 0;
}

MODULE_DEF(ext2_driver, ext2_init, ext2_finit, MODT_FS, "Miguel S.", ext2_ioctl);
