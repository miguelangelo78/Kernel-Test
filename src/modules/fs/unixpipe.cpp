/*
 * unixpipe.cpp
 *
 *  Created on: 15/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include <libc/ringbuffer.h>
#include "pipe.h"

static uint32_t read_unixpipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	unix_pipe_t * self = (unix_pipe_t *)node->device;
	size_t read = 0;

	while (read < size) {
		if (self->write_closed && !ring_buffer_unread(self->buffer))
			return read;
		size_t r = ring_buffer_read(self->buffer, 1, buffer + read);
		if (r && *((char *)(buffer + read)) == '\n')
			return read + r;
		read += r;
	}
	return read;
}

static uint32_t write_unixpipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	unix_pipe_t * self = (unix_pipe_t*)node->device;
	size_t written = 0;

	while (written < size) {
		if (self->read_closed) {
			/* SIGPIPE to current process */
			signal_t * sig = (signal_t*)malloc(sizeof(signal_t));
			task_t * curr_task = current_task_get();
			sig->handler = curr_task->signals.functions[SIGPIPE];
			sig->signum  = SIGPIPE;
			handle_signal((task_t *)curr_task, sig);
			return written;
		}
		written += ring_buffer_write(self->buffer, 1, buffer + written);
	}
	return written;
}

static uint32_t close_read_pipe(FILE * node) {
	unix_pipe_t * self = (unix_pipe_t*)node->device;
	self->read_closed = 1;
	if (!self->write_closed)
		ring_buffer_interrupt(self->buffer);
	return 0;
}

static uint32_t close_write_pipe(FILE * node) {
	unix_pipe_t * self = (unix_pipe_t*)node->device;
	self->write_closed = 1;
	if (!self->read_closed)
		ring_buffer_interrupt(self->buffer);
	return 0;
}

static int make_unix_pipe_(FILE ** pipes) {
	size_t size = UNIX_PIPE_BUFFER_SIZE;

	pipes[0] = (FILE*)malloc(sizeof(FILE));
	pipes[1] = (FILE*)malloc(sizeof(FILE));

	memset(pipes[0], 0, sizeof(FILE));
	memset(pipes[1], 0, sizeof(FILE));

	sprintf(pipes[0]->name, "%s", "[pipe:read]");
	sprintf(pipes[1]->name, "%s", "[pipe:write]");

	pipes[0]->flags = FS_PIPE;
	pipes[1]->flags = FS_PIPE;

	pipes[0]->read = read_unixpipe;
	pipes[1]->write = write_unixpipe;

	pipes[0]->close = close_read_pipe;
	pipes[1]->close = close_write_pipe;

	unix_pipe_t * internals = (unix_pipe*)malloc(sizeof(unix_pipe_t));
	internals->read_end = pipes[0];
	internals->write_end = pipes[1];
	internals->read_closed = 0;
	internals->write_closed = 0;
	internals->buffer = ring_buffer_create(size);

	pipes[0]->device = internals;
	pipes[1]->device = internals;

	return 0;
}

/************************************************/
/**** Module functions (init, finit, ioctl): ****/
/************************************************/
static int unix_pipe_mod_init(void) {
	/* Nothing to run */
	return 0;
}

static int unix_pipe_mod_finit(void) {
	return 0;
}

static uintptr_t unix_pipe_mod_ioctl(void * ioctl_packet) {
	SWITCH_IOCTL(ioctl_packet) {
		case 0: return (uintptr_t)make_unix_pipe_((FILE**)((uintptr_t*)ioctl_packet)[1]);
	}
	return 0;
}

MODULE_DEF(pipe_driver, unix_pipe_mod_init, unix_pipe_mod_finit, MODT_FS, "Miguel S.", unix_pipe_mod_ioctl);
