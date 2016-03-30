/*
 * cmos.cpp
 *
 *  Created on: 29/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <libc/hashmap.h>
#include <time.h>

/* Information: http://wiki.osdev.org/CMOS | http://wiki.osdev.org/RTC | http://stanislavs.org/helppc/cmos_ram.html */

#define CMOS_MEM_SIZE 128
#define CMOS_ADDR 0x70
#define NMI_disable_bit 1

/********************************* CMOS FUNCTIONS *********************************/
static uint8_t cmos_mem[CMOS_MEM_SIZE];

static uint8_t * cmos_cmd(uint8_t start_addr, uint8_t end_addr, uint8_t * data, int datalen) {
	uint8_t start = start_addr == -1 ? 0 : start_addr;
	uint8_t end = end_addr == -1 ? CMOS_MEM_SIZE : end_addr;
	int data_ctr = 0;
	for(int i = start; i < end_addr; i++) {
		outb(CMOS_ADDR, (NMI_disable_bit << 7) | i);
		if(!datalen) {
			cmos_mem[i] = inb(CMOS_ADDR + 1);
		} else {
			outb(CMOS_ADDR + 1, data[data_ctr++]);
			if(data_ctr >= datalen) break;
		}
	}
	return cmos_mem;
}

static uint8_t * cmos_read(void) {
	return cmos_cmd(-1,-1, 0, 0);
}

static uint8_t cmos_read(uint8_t cmos_register) {
	return cmos_cmd(cmos_register, cmos_register + 1, 0, 0)[cmos_register];
}

static uint8_t * cmos_write(uint8_t * data, int datalen) {
	return cmos_cmd(-1,-1, data, datalen);
}

static uint8_t * cmos_write(uint8_t cmos_register, uint8_t data) {
	return cmos_cmd(cmos_register, cmos_register + 1, &data, 1);
}

static uint8_t * cmos_write(uint8_t start_addr, uint8_t end_addr, uint8_t * data, int datalen) {
	return cmos_cmd(start_addr, end_addr, data, datalen);
}

/********************************* TIME/DATE/RTC FUNCTIONS *********************************/
#define bcd_to_bin(val) ((val / 16) * 10 + (val & 0xf))

static time_t rtc_time;
static uint32_t boot_time = 0;

static time_t * rtc_get_time(void) {
	uint8_t * cmos_data = cmos_cmd(0, 9, 0, 0);
	rtc_time.sec = bcd_to_bin(cmos_data[0]);
	rtc_time.min = bcd_to_bin(cmos_data[2]);
	rtc_time.hour = bcd_to_bin(cmos_data[4]);
	rtc_time.weekday = bcd_to_bin(cmos_data[6]);
	rtc_time.monthday =  bcd_to_bin(cmos_data[7]);
	rtc_time.month = bcd_to_bin(cmos_data[8]);
	rtc_time.year = bcd_to_bin(cmos_data[9]);
	return &rtc_time;
}

static uint32_t rtc_get_time_secs(void) {
	uint8_t * cmos_data = cmos_cmd(0, 9, 0, 0);
	uint32_t time = time_secs_of_years(bcd_to_bin(cmos_data[9]) - 1) +
		time_secs_of_month(bcd_to_bin(cmos_data[8]) - 1, bcd_to_bin(cmos_data[9])) +
		(bcd_to_bin(cmos_data[7]) - 1) * 86400 +
		(bcd_to_bin(cmos_data[4])) * 3600 +
		(bcd_to_bin(cmos_data[2])) * 60 +
		bcd_to_bin(cmos_data[0]) +
		0;
	return time;
}

static uint32_t now(void) {
	uintptr_t timer_ticks;
	MOD_IOCTLD("pit_driver", timer_ticks, 3);
	return boot_time + timer_ticks;
}

/********************************* RTC IRQ FUNCTIONS *********************************/
hashmap_t * rtc_cbacks;
int rtc_cback_count = 0;
FDECLV(hashmap_get_i, hashmap_get_i_t, void*, hashmap_t*, int);

enum CMOS_STATUS_REGS {
	RTC_STATUS_A = 0xA, RTC_STATUS_B = 0xB, RTC_STATUS_C = 0xC, RTC_STATUS_D = 0xD, CMOS_STATUS_DIAG = 0xE, CMOS_STATUS_SHUTDOWN = 0xF
};

static uintptr_t rtc_install_cback(char * func_name, uintptr_t address) {
	if(hashmap_has(rtc_cbacks, (char*)func_name)) return IOCTL_NULL;
	rtc_cback_count++;
	return (uintptr_t)hashmap_set(rtc_cbacks, (char*)func_name, (void*)address);
}

static uintptr_t rtc_uninstall_cback(char * func_name) {
	if(!rtc_cback_count) return IOCTL_NULL;
	rtc_cback_count--;
	return (uintptr_t)hashmap_remove(rtc_cbacks, (char*)func_name);
}

static void rtc_irq_ack(void) {
	cmos_read(RTC_STATUS_C);
}

static void rtc_enable_irq(void) {
	cmos_write(RTC_STATUS_B, (cmos_read(RTC_STATUS_B) & 0xF0) | 0x40);
}

static void rtc_handler(void) {
	for(int i = 0; i < rtc_cback_count; i++)
		FCASTF(hashmap_get_i(rtc_cbacks, i), void, void)();
	rtc_irq_ack();
}

static void rtc_set_frequency(char rate) {
	if(rate < 0) rate = 0;
	if(rate > 15) rate = 15;
	cmos_write(RTC_STATUS_A, (cmos_read(RTC_STATUS_A) & 0xF0) | ((0xF - rate) & 0xF));
	rtc_enable_irq();
}

/********************************* INITIALIZATION AND IOCTL *********************************/
static int cmos_init(void) {
	hashmap_get_i = (hashmap_get_i_t)SYF((char*)"hashmap_get_i");
	rtc_cbacks = hashmap_create(1);
	SYA(irq_install_handler, Kernel::CPU::IRQ::IRQ_CMOS, rtc_handler);
	rtc_set_frequency(0);
	boot_time = rtc_get_time_secs();
	return 0;
}

static int cmos_finit(void) {
	return 0;
}

static uintptr_t cmos_ioctl(void * data) {
	uintptr_t *d = (uintptr_t*)data;
	switch(d[0]) {
	case 1:
		rtc_install_cback((char*)d[1], d[2]);
		break;
	case 2:
		rtc_uninstall_cback((char*)d[1]);
		break;
	case 3:
		return (uintptr_t)rtc_get_time();
	case 4:
		return rtc_get_time_secs();
	case 5:
		return now();
	case 7:
		return boot_time;
	}
	return 0;
}

MODULE_DEF(cmos_driver, cmos_init, cmos_finit, MODT_CLOCK, "Miguel S.", cmos_ioctl);
