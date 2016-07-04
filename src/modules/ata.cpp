/*
 * ata.cpp
 *
 *  Created on: 02/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include <fs.h>
#include "ata.h"

#define ATA_SECTOR_SIZE 512

typedef struct {
	int io_base;
	int control;
	int slave;
	ata_identify_t identity;
} ata_dev_t;

static spin_lock_t ata_lock = { 0 };

static ata_dev_t ata_primary_master     = {0x1F0, 0x3F6, 0};
static ata_dev_t ata_primary_slave      = {0x1F0, 0x3F6, 1};
static ata_dev_t ata_secondary_master   = {0x170, 0x376, 0};
static ata_dev_t ata_secondary_slave    = {0x170, 0x376, 1};

/************* ATA Virtual Filesystem Functions *************/
static char ata_drive_char = 'a';

static uint64_t ata_max_offset(ata_dev_t * dev) {

}

static uint32_t read_ata(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {

}

static uint32_t write_ata(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {

}

static void open_ata(FILE * node, unsigned int flags) {

}

static void close_ata(FILE * node) {

}

static FILE * ata_device_create(ata_dev_t * dev) {

}

/************* ATA Sector Read/Write Functions *************/
/* Prototype: */
static int ata_wait(ata_dev_t * dev, int advanced);

static void ata_device_read_sector(ata_dev_t * dev, uint32_t lba, uint8_t * buff) {
	spin_lock(ata_lock);

	int errors = 0;
try_again:

	outb(dev->io_base + ATA_REG_CONTROL, 0x02);
	ata_wait(dev, 0);

	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xE0 | dev->slave << 4 | (lba & 0x0F000000) >> 24);
	outb(dev->io_base + ATA_REG_FEATURES, 0x00);
	outb(dev->io_base + ATA_REG_SECCOUNT0, 1);
	outb(dev->io_base + ATA_REG_LBA0, (lba & 0x000000FF) >>  0);
	outb(dev->io_base + ATA_REG_LBA1, (lba & 0x0000FF00) >>  8);
	outb(dev->io_base + ATA_REG_LBA2, (lba & 0x00FF0000) >> 16);
	outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	if(ata_wait(dev, 1)) {
		kprintf("\n\t!Error during ATA read of lba block %d", lba);
		if(++errors > 4) {
			kprintf("\n\t!! Too many errors trying to read this block. Bailing. !!");
			spin_unlock(ata_lock);
			return;
		}
		goto try_again;
	}

	/* Read from sector: */
	int size = 256;
	insm(dev->io_base, buff, size);
	ata_wait(dev, 0);

	spin_unlock(ata_lock);
}

static void ata_device_write_sector(ata_dev_t * dev, uint32_t lba, uint8_t * buff) {
	spin_lock(ata_lock);

	outb(dev->io_base + ATA_REG_CONTROL, 0x02);

	ata_wait(dev, 0);
	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xE0 | dev->slave << 4 | (lba & 0x0F000000) >> 24);
	ata_wait(dev, 0);

	outb(dev->io_base + ATA_REG_FEATURES, 0x00);
	outb(dev->io_base + ATA_REG_SECCOUNT0, 0x01);
	outb(dev->io_base + ATA_REG_LBA0, (lba & 0x000000FF) >>  0);
	outb(dev->io_base + ATA_REG_LBA1, (lba & 0x0000FF00) >>  8);
	outb(dev->io_base + ATA_REG_LBA2, (lba & 0x00FF0000) >> 16);
	outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
	ata_wait(dev, 0);

	/* Write into sector: */
	int size = ATA_SECTOR_SIZE / 2;
	outsm(dev->io_base, buff, size);
	outb(dev->io_base + 0x07, ATA_CMD_CACHE_FLUSH);
	ata_wait(dev, 0);

	spin_unlock(ata_lock);
}

static int buffer_compare(uint32_t * buff1, uint32_t * buff2, size_t size) {
	size_t i = 0;
	if(i < size) {
		if(*buff1 != *buff2) return 1;
		buff1++;
		buff2++;
		i += sizeof(uint32_t);
	}
	return 0;
}

static void ata_device_write_retry(ata_dev_t * dev, uint32_t lba, uint8_t * buff) {
	uint8_t * read_buff = (uint8_t*)malloc(ATA_SECTOR_SIZE);
	IRQ_OFF();
	do {
		ata_device_write_sector(dev, lba, buff);
		ata_device_read_sector(dev, lba, read_buff);
	} while(buffer_compare((uint32_t *)buff, (uint32_t*)read_buff, ATA_SECTOR_SIZE));
	IRQ_RES();
	free(read_buff);
}


/************* ATA Device IOCTL + Identify Functions *************/
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

static char ata_device_init(ata_dev_t * dev) {
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
		return 1;
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
	return 0;
}

static char ata_device_detect(ata_dev_t * dev) {
	ata_soft_reset(dev);
	ata_io_wait(dev);
	outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
	ata_io_wait(dev);
	ata_status_wait(dev, 10000);

	unsigned char cl = inb(dev->io_base + ATA_REG_LBA1);
	unsigned char ch = inb(dev->io_base + ATA_REG_LBA2);

	kprintf("\n\tDevice detected - 0x%2x 0x%2x", cl, ch);
	if(cl == 0xFF && ch == 0xFF) return 1;

	if((cl == 0x00 && ch == 0x00) || (cl == 0x3C && ch == 0xC3)) {
		/* Found Parallel ATA device, or emulated SATA */

		/* Create VFS node and mount it */

		/* Initialize ata device */
		return ata_device_init(dev);
	}
	return 1;
}


/************* ATA Initializers and IOCTL *************/
static int ata_init(void) {
	kprintf("\n\t>> ATA:");
	if(!ata_device_detect(&ata_primary_master))   kprintf("\n\t\t- !Mounted!");
	if(!ata_device_detect(&ata_primary_slave))    kprintf("\n\t\t- !Mounted!");
	if(!ata_device_detect(&ata_secondary_master)) kprintf("\n\t\t- !Mounted!");
	if(!ata_device_detect(&ata_secondary_slave))  kprintf("\n\t\t- !Mounted!");
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
