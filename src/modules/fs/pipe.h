/*
 * pipe.h
 *
 *  Created on: 15/07/2016
 *      Author: Miguel
 */

#ifndef SRC_MODULES_FS_PIPE_H_
#define SRC_MODULES_FS_PIPE_H_

#include <stdint.h>
#include <libc/list.h>
#include <fs.h>
#include <module.h>

typedef struct _pipe_device {
	uint8_t * buffer;
	size_t write_ptr;
	size_t read_ptr;
	size_t size;
	size_t refcount;
	volatile int lock_read[2];
	volatile int lock_write[2];
	list_t * wait_queue_readers;
	list_t * wait_queue_writers;
	int dead;
} pipe_device_t;

typedef struct unix_pipe {
	FILE * read_end;
	FILE * write_end;

	volatile int read_closed;
	volatile int write_closed;

	ring_buffer_t * buffer;
} unix_pipe_t;

inline FILE * make_pipe(size_t size) {
	FILE * ret;
	MOD_IOCTLDT("pipe_driver", FILE*, ret, 0, (uintptr_t)size);
	return ret;
}

inline int make_unix_pipe(FILE ** pipes) {
	int ret;
	MOD_IOCTLD("pipe_driver", ret, 0, (uintptr_t)pipes);
	return ret;
}

inline int pipe_size(FILE * node) {
	int ret;
	MOD_IOCTLD("pipe_driver", ret, 1, (uintptr_t)node);
	return ret;
}

inline int pipe_unsize(FILE * node) {
	int ret;
	MOD_IOCTLD("pipe_driver", ret, 2, (uintptr_t)node);
	return ret;
}

#endif /* SRC_MODULES_FS_PIPE_H_ */
