/*
 * serial.h
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#ifndef SRC_SERIAL_H_
#define SRC_SERIAL_H_

enum COMS {
	COM1 = 0x3F8
};

class Serial {
public:
	Serial();

	void init(char port);
	void write(char byte);
	void puts(char * str);
	void send(char * buffer, int buffer_size);
	void printf(char * fmt, ...);
private:
	char is_transmit_empty(void);
};

#endif /* SRC_SERIAL_H_ */
