/*
 * serial.h
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#ifndef SRC_SERIAL_H_
#define SRC_SERIAL_H_

#include <stdint.h>
#include <arch/x86/cpu.h>
#include <fs.h>

/* See reference: http://wiki.osdev.org/Serial_Ports */

enum COMS {
	COM1 = 0x3F8,
	COM2 = 0x2F8,
	COM3 = 0x3E8,
	COM4 = 0x2E8
};

class Serial {
public:
	#define SERIAL_CBACK_BUFFER_SIZE 16
	#define SERIAL_READ_BUFF_SIZE 16

	Serial();
	void init(uint16_t port);
	void init_late(uint16_t port);

	void write(char byte);
	void puts(char * str);
	void send(char * buffer, int buffer_size);
	void printf(char * fmt, ...);
	void printf(char * fmt, va_list args, char ign);

	char read_async(void);
	char read(void);
	uint8_t* read_buff(int bytecount);

	void flush(void);

	char is_ready(void);

	FILE * serial_pipe; /* This pipe will only be initialized once we loaded modules and the pipe module */
private:
	uint16_t comport; /* The actual address of the com port */
	uint16_t port;
	char is_tx_empty(void);
	char is_rx_empty(void);
	char pop(void);
	friend int serial_cback(Kernel::CPU::regs_t * regs);
	uint8_t serial_cback_buffer[SERIAL_CBACK_BUFFER_SIZE]; /* One buffer to be used for interrupts */
	uint8_t serial_read_buff[SERIAL_READ_BUFF_SIZE]; /* And another one in case the interrupts are disabled */
};

#endif /* SRC_SERIAL_H_ */
