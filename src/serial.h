/*
 * serial.h
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#ifndef SRC_SERIAL_H_
#define SRC_SERIAL_H_

#include <stdint.h>

enum COMS {
	COM1 = 0x3F8
};

class Serial {
public:
	Serial();

	void init(uint16_t port);
	void write(char byte);
	void puts(char * str);
	void send(char * buffer, int buffer_size);
	void printf(char * fmt, ...);

	char read_async(void);
	char read(void);
	char* read_buff(int bytecount);

private:
	uint16_t port;
	char is_tx_empty(void);
	char is_rx_empty(void);
};

#endif /* SRC_SERIAL_H_ */
