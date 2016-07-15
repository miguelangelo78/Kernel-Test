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

#define UNIX_PIPE_BUFFER_SIZE 512

static uint32_t read_unixpipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	return 0;
}

static uint32_t write_unixpipe(FILE * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	return 0;
}

static uint32_t close_read_pipe(FILE * node) {
	return 0;
}

static uint32_t close_write_pipe(FILE * node) {
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
	// TODO: internals->buffer = ring_buffer_create(size);

	pipes[0]->device = internals;
	pipes[1]->device = internals;

	return 0;
}

/************************************************/
/**** Module functions (init, finit, ioctl): ****/
/************************************************/
static int unix_pipe_mod_init(void) {
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
