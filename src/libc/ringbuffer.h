/*
 * ringbuffer.h
 *
 *  Created on: 15/07/2016
 *      Author: Miguel
 */

#ifndef SRC_LIBC_RINGBUFFER_H_
#define SRC_LIBC_RINGBUFFER_H_

#include <stdint.h>
#include <libc/list.h>
#include <fs.h>

typedef struct {
	unsigned char * buffer;
	size_t write_ptr;
	size_t read_ptr;
	size_t size;
	volatile int lock[2];
	list_t * wait_queue_readers;
	list_t * wait_queue_writers;
	int internal_stop;
} ring_buffer_t;

#endif /* SRC_LIBC_RINGBUFFER_H_ */
