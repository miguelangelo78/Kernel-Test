/*
 * pit.cpp
 *
 *  Created on: 29/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <libc/hashmap.h>
#include <time.h>

/* Information: http://wiki.osdev.org/Programmable_Interval_Timer */

#define PIT_CALLBACK_SERVICE_MAX 16 /* Maximum amount of callbacks that the pit can serve */

#define PIT_DEFAULT_HZ 1000
#define PIT_CLOCK 1193180
#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL_COUNT 3
#define RESYNC_TIME 1

static char pit_servicing = 1;
static hashmap_t * pit_cbacks = 0;
static volatile uint16_t pit_cback_count = 0;
static uint32_t current_hz = 0;
static uint32_t ticks = 0;
static uint32_t subticks = 0;
char ** services_names;
uint16_t next_available_service = 0;
FDECLV(hashmap_get, hashmap_get_t, void*, hashmap_t * , void *);

enum PIT_CHANNEL {
	PIT_CHANNEL_0, PIT_CHANNEL_1, PIT_CHANNEL_2, PIT_CHANNEL_READBACK
};

enum PIT_ACCESS {
	PIT_ACC_LATCH, PIT_ACC_LO, PIT_ACC_HI, PIT_ACC_LOHI
};

enum PIT_OPERATIONMODE {
	PIT_M_INTC, PIT_M_OSHOT, PIT_M_RATEGEN, PIT_M_SQW, PIT_M_SOFT, PIT_M_HARD, PIT_M_RATEGEN1, PIT_M_SQW1
};

enum PIT_DATAMODE {
	PIT_DAT_BIN, PIT_DAT_BCD
};

enum PIT_READCMD {
	PIT_READ_RESERVED, PIT_READ_PIT_A, PIT_READ_PIT_B, PIT_RAD_PIT_CM, PIT_READ_LATCH_STATUS, PIT_READ_LATCH_COUNT
};

enum PIT_STATUS {
	PIT_STATUS_DATA_MODE, PIT_STATUS_OP_MODE, PIT_STATUS_ACCESS_MODE, PIT_STATUS_NULLCOUNT, PIT_STATUS_OUTPIN
};

static int pit_cmd(char channel, char access_mode, char operation_mode, char data_mode, int send_data, char is_reading) {
	int io_port = 0;
	if(is_reading) io_port = PIT_CMD_PORT - (PIT_CHANNEL_COUNT - (operation_mode & 0x3));
	else io_port = PIT_CMD_PORT - (PIT_CHANNEL_COUNT - (channel & 0x3));

	if(io_port == PIT_CMD_PORT && !is_reading) return IOCTL_NULL;

	char cmd = (data_mode & 0x1) | ((channel & 0x3) << 6) | ((operation_mode & 0x7) << 1) | ((access_mode & 0x3) << 4);
	outb(PIT_CMD_PORT, cmd);
	if(!is_reading) {
		outb(io_port, send_data & 0xFF);
		outb(io_port, (send_data & 0xFF00) >> 8);
	} else {
		int read_data = inb(io_port);
		read_data |= inb(io_port) << 8;
		return read_data;
	}
	return IOCTL_NULL;
}

static char pit_cmd(char channel, char access_mode, char operation_mode, char data_mode, int send_data) {
	return pit_cmd(channel, access_mode, operation_mode, data_mode, send_data, 0);
}

static int pit_read_cmd(char channel_read, char latch_count, char latch_status) {
	return pit_cmd(PIT_CHANNEL_READBACK, (!(latch_count & 1) << 1) | !(latch_status & 1), channel_read, 0, -1, 1);
}

static int pit_read(char channel) {
	return pit_read_cmd(channel, 1, 0);
}

static int pit_read_status(char channel) {
	return pit_read_cmd(channel, 0, 1);
}

static uintptr_t pit_get_subticks(void) {
	return subticks;
}

static uintptr_t pit_get_ticks(void) {
	return ticks;
}

static void pit_sethz(int hz) {
	if(hz >= 1 && hz < PIT_CLOCK) {
		current_hz = hz;
		pit_cmd(PIT_CHANNEL_0, PIT_ACC_LOHI, PIT_M_SQW, PIT_DAT_BIN, PIT_CLOCK / hz);
	}
}

static void pit_handler(void) {
	if(++subticks == current_hz) {
		ticks++;
		subticks = 0;
	}

	if(!pit_servicing) return;

	for(int i = 0; i < pit_cback_count; i++) {
		uintptr_t addr = (uintptr_t)hashmap_get(pit_cbacks, services_names[0]);
		if(addr)
			FCASTF(addr, void, void)();
	}
}

static uintptr_t pit_install_cback(char * func_name, uintptr_t address) {
	if(hashmap_has(pit_cbacks, (char*)func_name) || pit_cback_count >= PIT_CALLBACK_SERVICE_MAX) return IOCTL_NULL;
	uintptr_t ret = (uintptr_t)hashmap_set(pit_cbacks, (char*)func_name, (void*)address);
	services_names[next_available_service] = (char*)malloc(strlen(func_name) + 1);
	strcpy(services_names[next_available_service], func_name);

	/* Find the next available service: */
	for(int i = 0; i < PIT_CALLBACK_SERVICE_MAX; i++)
		if(!services_names[i]) {
			next_available_service = i;
			break;
		}
	pit_cback_count++;
	return ret;
}

static uintptr_t pit_uninstall_cback(char * func_name) {
	if(!pit_cback_count) return IOCTL_NULL;
	for(int i = 0; i < pit_cback_count; i++)
		if(!strcmp(services_names[i], func_name)) {
			free(services_names[i]);
			services_names[i] = 0;
			next_available_service = i;
			pit_cback_count--;
			break;
		}

	return (uintptr_t)hashmap_remove(pit_cbacks, (char*)func_name);
}

static void relative_time(uint32_t seconds, uint32_t subseconds, uint32_t * out_seconds, uint32_t * out_subseconds) {
	if (subseconds + subticks > current_hz) {
		if(out_seconds)
			*out_seconds    = ticks + seconds + 1;
		if(out_subseconds)
			*out_subseconds = (subseconds + subticks) - current_hz;
	} else {
		if(out_seconds)
			*out_seconds    = ticks + seconds;
		if(out_subseconds)
			*out_subseconds = subticks + subseconds;
	}
}

static int pit_init(void) {
	hashmap_get = (hashmap_get_t)SYF((char*)"hashmap_get");
	/* Each service has a name, and we use this to iterate the hashmap: */
	services_names = (char**)malloc(PIT_CALLBACK_SERVICE_MAX * sizeof(char**));
	for(int i = 0;i < PIT_CALLBACK_SERVICE_MAX; i++)
		services_names[i] = 0;
	pit_cbacks = hashmap_create(PIT_CALLBACK_SERVICE_MAX);
	pit_sethz(PIT_DEFAULT_HZ);
	SYA(irq_install_handler, Kernel::CPU::IRQ::IRQ_PIT, pit_handler);
	return 0;
}

static int pit_finit(void) {
	return 1;
}

static uintptr_t pit_ioctl(void * data) {
	uintptr_t *d = (uintptr_t*)data;
	switch(d[0]) {
	case 1:
		pit_install_cback((char*)d[1], d[2]);
		break;
	case 2:
		pit_uninstall_cback((char*)d[1]);
		break;
	case 3:
		return pit_get_ticks();
	case 4:
		return pit_get_subticks();
	case 5:
		return (pit_servicing = (char)d[1]);
	}
	return 0;
}

MODULE_DEF(pit_driver, pit_init, pit_finit, MODT_CLOCK, "Miguel S.", pit_ioctl);
