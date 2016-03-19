/*
 * serial.cpp
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#include <serial.h>
#include <io.h>
#include <va_list.h>
#include <libc.h>
#include <system.h>

using namespace Kernel::IO;

Serial::Serial(void) { port = 0; }

int serial_cback_ctr = 0;

int serial_cback(Kernel::CPU::regs_t * regs) {
	if(serial_cback_ctr >= SERIAL_CBACK_BUFFER_SIZE) Kernel::serial.pop(); /* Buffer is full */
	Kernel::serial.serial_cback_buffer[serial_cback_ctr++] = Kernel::serial.read(); /* Store character */
	return 0;
}

void Serial::init(uint16_t port) {
	size_t irq_num = 0;

	memset(serial_cback_buffer, 0, SERIAL_CBACK_BUFFER_SIZE);

	switch(port) {
	case COM1: irq_num = 4; break;
	case COM2: irq_num = 3; break;
	case COM3: case COM4: break; /* COM3 and COM4 do not trigger interrupts */
	default: return;
	}

	this->port = port;
	outb(port + 1, 0x00);
	outb(port + 3, 0x80);
	outb(port, 0x03);
	outb(port + 1, 0x00);
	outb(port + 3, 0x03);
	outb(port + 2, 0xC7);
	outb(port + 4, 0x0B);
	if(irq_num) {
		outb(port + 1, 0x01); /* Enable interrupt */
		Kernel::CPU::IRQ::irq_install_handler(irq_num, serial_cback);
	}
	read_async();
}

void Serial::write(char byte) {
	while(!is_tx_empty());
	outb(port, byte);
}

void Serial::puts(char * str) {
	while(*str) write(*str++);
}

void Serial::send(char * buffer, int buffer_size) {
	for(;buffer_size--;) *buffer++;
}

char serial_printf_buff[512];

void Serial::printf(char * fmt, ...) {
	strfmt(serial_printf_buff, fmt);
	puts(serial_printf_buff);
}

char Serial::read_async(void) {
	if(serial_cback_ctr) return pop(); /* Read from buffer instead */

	if(!is_rx_empty()) return 0;
	return inb(port);
}

char Serial::read(void) {
	if(serial_cback_ctr) return pop(); /* Read from buffer instead */

	while(!is_rx_empty());
	return inb(port);
}

uint8_t * Serial::read_buff(int bytecount) {
	if(serial_cback_ctr) return serial_cback_buffer; /* Return buffer instead */

	memset(serial_read_buff, 0, SERIAL_READ_BUFF_SIZE);
	for(int i = 0;i < bytecount; i++)
		serial_read_buff[i] = read();
	return serial_read_buff;
}

char Serial::is_tx_empty(void) {
	return inb(port + 5) & 0x20;
}

char Serial::is_rx_empty(void) {
	return inb(port + 5) & 1;
}

void Serial::flush(void) {
	serial_cback_ctr = 0;
	memset(serial_cback_buffer, 0, SERIAL_CBACK_BUFFER_SIZE);
}

char Serial::pop(void) {
	if(!serial_cback_ctr) { flush(); return 0; }
	serial_cback_ctr--;
	return serial_cback_buffer[0];
}

char Serial::is_ready(void) {
	return serial_cback_ctr > 0;
}
