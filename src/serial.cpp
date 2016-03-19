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

using namespace Kernel::IO;

Serial::Serial(void) { }

void Serial::init(char port) {
	outb(port + 1, 0x00);
	outb(port + 3, 0x80);
	outb(port, 0x03);
	outb(port + 1, 0x00);
	outb(port + 3, 0x03);
	outb(port + 2, 0xC7);
	outb(port + 4, 0x0B);
}

void Serial::write(char byte) {
	while(!is_transmit_empty());
	outb(COM1, byte);
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

char Serial::is_transmit_empty(void) {
	return inb(COM1+5) & 0x20;
}

