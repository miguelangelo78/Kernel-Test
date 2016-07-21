/*
 * pipe.cpp
 *
 *  Created on: 15/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include <time.h>
#include "pipe.h"

/******************************/
/**** Function prototypes: ****/
/******************************/
static uint32_t read_pipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer);
static uint32_t write_pipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer);
static uint32_t open_pipe(FILE * node, unsigned int flags);
static uint32_t close_pipe(FILE * node);

/****************************************/
/**** Pipe implementation functions: ****/
/****************************************/
static inline size_t pipe_unread(pipe_device_t * pipe) {
	if (pipe->read_ptr == pipe->write_ptr)
		return 0;
	if (pipe->read_ptr > pipe->write_ptr)
		return (pipe->size - pipe->read_ptr) + pipe->write_ptr;
	else
		return (pipe->write_ptr - pipe->read_ptr);
}

static int pipe_size_(FILE * node) {
	return pipe_unread((pipe_device_t *)node->device);
}

static inline size_t pipe_available(pipe_device_t * pipe) {
	if (pipe->read_ptr == pipe->write_ptr)
		return pipe->size - 1;

	if (pipe->read_ptr > pipe->write_ptr)
		return pipe->read_ptr - pipe->write_ptr - 1;
	else
		return (pipe->size - pipe->write_ptr) + pipe->read_ptr - 1;
}

static int pipe_unsize_(FILE * node) {
	return pipe_available((pipe_device_t *)node->device);
}

static inline void pipe_increment_read(pipe_device_t * pipe) {
	if (++pipe->read_ptr == pipe->size)
		pipe->read_ptr = 0;
}

static inline void pipe_increment_write(pipe_device_t * pipe) {
	if (++pipe->write_ptr == pipe->size)
		pipe->write_ptr = 0;
}

/******************************/
/**** Pipe CRUD functions: ****/
/******************************/
static uint32_t read_pipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	/* Retrieve the pipe object associated with this file node */
	pipe_device_t * pipe = (pipe_device_t *)node->device;

	if (pipe->dead) {
		send_signal(current_task_getpid(), SIGPIPE);
		return 0;
	}

	size_t collected = 0;
	while (collected == 0) {
		spin_lock(pipe->lock_read);
		while (pipe_unread(pipe) > 0 && collected < size) {
			buffer[collected] = pipe->buffer[pipe->read_ptr];
			pipe_increment_read(pipe);
			collected++;
		}
		spin_unlock(pipe->lock_read);
		wakeup_queue(pipe->wait_queue_writers);
		/* Deschedule and switch */
		if (collected == 0)
			sleep_on(pipe->wait_queue_readers);
	}
	return collected;
}

static uint32_t write_pipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	/* Retrieve the pipe object associated with this file node */
	pipe_device_t * pipe = (pipe_device_t *)node->device;

	if (pipe->dead) {
		send_signal(current_task_getpid(), SIGPIPE);
		return 0;
	}

	size_t written = 0;
	while (written < size) {
		spin_lock(pipe->lock_write);

		while (pipe_available(pipe) > 0 && written < size) {
			pipe->buffer[pipe->write_ptr] = buffer[written];
			pipe_increment_write(pipe);
			written++;
		}

		spin_unlock(pipe->lock_write);
		wakeup_queue(pipe->wait_queue_readers);
		if (written < size)
			sleep_on(pipe->wait_queue_writers);
	}
	return written;
}

/******************************/
/**** Pipe open and close: ****/
/******************************/
static uint32_t open_pipe(FILE * node, unsigned int flags) {
	/* Retrieve the pipe object associated with this file node */
	pipe_device_t * pipe = (pipe_device_t *)node->device;

	/* Add a reference */
	pipe->refcount++;
	return 0;
}

static uint32_t close_pipe(FILE * node) {
	/* Retreive the pipe object associated with this file node */
	pipe_device_t * pipe = (pipe_device_t *)node->device;

	/* Drop one reference */
	pipe->refcount--;
	return 0;
}

static FILE * make_pipe_(size_t size) {
	FILE * fnode = (FILE*)malloc(sizeof(FILE));
	pipe_device_t * pipe = (pipe_device_t*)malloc(sizeof(pipe_device_t));
	memset(fnode, 0, sizeof(FILE));

	fnode->device = 0;
	sprintf((char*)fnode->name, "%s", "[pipe]");
	fnode->uid   = 0;
	fnode->gid   = 0;
	fnode->flags = FS_PIPE;
	fnode->read  = read_pipe;
	fnode->write = write_pipe;
	fnode->open  = open_pipe;
	fnode->close = close_pipe;
	fnode->readdir = 0;
	fnode->finddir = 0;
	fnode->ioctl   = 0;
	fnode->get_size = pipe_size;

	fnode->atime = now();
	fnode->mtime = fnode->atime;
	fnode->ctime = fnode->atime;

	fnode->device = pipe;

	pipe->buffer    = (uint8_t*)malloc(size);
	pipe->write_ptr = 0;
	pipe->read_ptr  = 0;
	pipe->size      = size;
	pipe->refcount  = 0;
	pipe->dead      = 0;

	spin_init(pipe->lock_read);
	spin_init(pipe->lock_write);

	pipe->wait_queue_writers = list_create();
	pipe->wait_queue_readers = list_create();

	return fnode;
}

/************************************************/
/**** Module functions (init, finit, ioctl): ****/
/************************************************/
static int pipe_mod_init(void) {
	return 0;
}

static int pipe_mod_finit(void) {
	return 0;
}

static uintptr_t pipe_mod_ioctl(void * ioctl_packet) {
	SWITCH_IOCTL(ioctl_packet) {
		case 0: return (uintptr_t)make_pipe_((size_t)((uintptr_t*)ioctl_packet)[1]);
		case 1: return (uintptr_t)pipe_size_((FILE*)((uintptr_t*)ioctl_packet)[1]);
		case 2: return (uintptr_t)pipe_unsize_((FILE*)((uintptr_t*)ioctl_packet)[1]);
	}
	return 0;
}

MODULE_DEF(pipe_driver, pipe_mod_init, pipe_mod_finit, MODT_FS, "Miguel S.", pipe_mod_ioctl);
