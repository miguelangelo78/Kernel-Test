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

void Serial::init(uint16_t port) {
	this->port = port;
	outb(port + 1, 0x00);
	outb(port + 3, 0x80);
	outb(port, 0x03);
	outb(port + 1, 0x00);
	outb(port + 3, 0x03);
	outb(port + 2, 0xC7);
	outb(port + 4, 0x0B);

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
	if(!is_rx_empty()) return 0;
	return inb(port);
}

char Serial::read(void) {
	while(!is_rx_empty());
	return inb(port);
}

#define SERIAL_READ_BUFF_SIZE 16
char serial_read_buff[SERIAL_READ_BUFF_SIZE];

char * Serial::read_buff(int bytecount) {
	memset(serial_read_buff, 0, SERIAL_READ_BUFF_SIZE);
	for(int i = 0;i < SERIAL_READ_BUFF_SIZE; i++)
		serial_read_buff[i] = read();
	return serial_read_buff;
}

char Serial::is_tx_empty(void) {
	return inb(port + 5) & 0x20;
}

char Serial::is_rx_empty(void) {
	return inb(port + 5) & 1;
}

