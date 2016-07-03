/*
 * ata.cpp
 *
 *  Created on: 02/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include "ata.h"

typedef struct {
	int io_base;
	int control;
	int slave;
	ata_identify_t identity;
} ata_dev_t;

static spin_lock_t ata_lock = { 0 };

static ata_dev_t ata_primary_master     = {0x1F0, 0x3F6, 0};
static ata_dev_t ata_primary_slave 	    = {0x1F0, 0x3F6, 1};
static ata_dev_t ata_secondary_master   = {0x170, 0x376, 0};
static ata_dev_t ata_secondary_slave    = {0x170, 0x376, 1};

/************* ATA IOCTL Functions *************/
static void ata_io_wait(ata_dev_t * dev) {
	inb(dev->io_base + ATA_REG_ALTSTATUS);
	inb(dev->io_base + ATA_REG_ALTSTATUS);
	inb(dev->io_base + ATA_REG_ALTSTATUS);
	inb(dev->io_base + ATA_REG_ALTSTATUS);
}

static int ata_status_wait(ata_dev_t * dev, int timeout) {
	int status;
	if(timeout > 0) {
		int i = 0;
		while(((status = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY) && (i < timeout)) i++;
	} else {
		while((status = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY);
	}
	return status;
}

static int ata_wait(ata_dev_t * dev, int advanced) {
	uint8_t status = 0;

	ata_io_wait(dev);
	status = ata_status_wait(dev, -1);

	if(advanced) {
		status = inb(dev->io_base + ATA_REG_STATUS);
		if(status   & ATA_SR_ERR)  return 1;
		if(status   & ATA_SR_DF)   return 1;
		if(!(status & ATA_SR_DRQ)) return 1;
	}
	return 0;
}

static void ata_soft_reset(ata_dev_t * dev) {
	outb(dev->control, 0x04);
	ata_io_wait(dev);
	outb(dev->control, 0x00);
}

static void ata_device_init(ata_dev_t * dev) {
	kprintf("\n\t\t> Initializing IDE device on bus %d", dev->io_base);

	outb(dev->io_base + 1, 1);
	outb(dev->control, 0);

	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
	ata_io_wait(dev);

	outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	ata_io_wait(dev);

	int status = inb(dev->io_base + ATA_REG_COMMAND);
	kprintf("\n\t\t- Device status: %d", status);

	ata_wait(dev, 0);

	if(status == 0) {
		kprintf("\n\t\t!Not a hard disk!");
		return;
	}

	uint16_t * buff = (uint16_t*)&dev->identity;
	for(int i = 0; i < 256; i++)
		buff[i] = ins(dev->io_base);

	uint8_t * ptr = (uint8_t*)&dev->identity.model;
	for(int i = 0; i < 39; i += 2) {
		uint8_t tmp = ptr[i + 1];
		ptr[i + 1] = ptr[i];
		ptr[i] = tmp;
	}

	kprintf("\n\t\t- Device name: %s", dev->identity.model);
	kprintf("\n\t\t- Sectors (48): %d", (uint32_t)dev->identity.sectors_48);
	kprintf("\n\t\t- Sectors (24): %d", dev->identity.sectors_28);

	outb(dev->io_base + ATA_REG_CONTROL, 0x02);
}

static int ata_device_detect(ata_dev_t * dev) {
	ata_soft_reset(dev);
	ata_io_wait(dev);
	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
	ata_io_wait(dev);
	ata_status_wait(dev, 10000);

	unsigned char cl = inb(dev->io_base + ATA_REG_LBA1);
	unsigned char ch = inb(dev->io_base + ATA_REG_LBA2);

	kprintf("\n\tDevice detected - 0x%2x 0x%2x", cl, ch);
	if(cl == 0xFF && ch == 0xFF) return 0;

	if((cl == 0x00 && ch == 0x00) || (cl == 0x3C && ch == 0xC3)) {
		/* Found Parallel ATA device, or emulated SATA */

		/* Create VFS node and mount it */

		/* Initialize ata device */
		ata_device_init(dev);
		return 1;
	}
	return 0;
}


/************* ATA Initializers and IOCTL *************/
static int ata_init(void) {
	kprintf("\n\t>> ATA:");
	ata_device_detect(&ata_primary_master);
	ata_device_detect(&ata_primary_slave);
	ata_device_detect(&ata_secondary_master);
	ata_device_detect(&ata_secondary_slave);
	kprintf("\n");
	return 0;
}

static int ata_finit(void) {
	return 0;
}

static uintptr_t ata_ioctl(void * ioctl_packet) {
	return 0;
}

MODULE_DEF(ext2_driver, ata_init, ata_finit, MODT_STORAGE, "Miguel S.", ata_ioctl);
