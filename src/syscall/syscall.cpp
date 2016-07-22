/*
 * syscall.cpp
 *
 *  Created on: 15 May 2016
 *      Author: miguel
 */
#include <system.h>
#include <module.h>
#include <libc/hashmap.h>
#include <libc/list.h>
#include "syscall_nums.h"
#include <time.h>
#include <utsname.h>

extern int (*syscalls[])();
extern uint32_t num_syscalls;

namespace Kernel {
namespace Syscall {

/************************************************/
/********** System Call Implementation **********/
/************************************************/
struct syscall_packet {
	char * syscall_name;
	int no;
	uintptr_t syscall_addr;
};

uint32_t syscall_count = 0;
syscall_callback_t * syscall_vector;
hashmap_t * syscall_vector_hash;
char syscall_initialized = 0;
list_t * syscall_schedule_installs = 0;

void syscall_handler(Kernel::CPU::regs_t * regs) {
	regs->eax--;
	if(regs->eax >= SYSCALL_MAXCALLS) return;

	syscall_callback_t cback = syscall_vector[regs->eax];
	if(!cback) return;

	current_task->syscall_regs = regs;

	/* Run system call: */
	uint32_t ret = cback(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

	if ((current_task->syscall_regs == regs) ||
			((uintptr_t)cback != (uintptr_t)&fork && (uintptr_t)cback != (uintptr_t)&task_clone)) {
		regs->eax = ret;
	}
}

void syscalls_initialize(void) {
	syscall_vector = new syscall_callback_t[SYSCALL_MAXCALLS];
	memset(syscall_vector, 0, SYSCALL_MAXCALLS);
	syscall_vector_hash = hashmap_create(SYSCALL_MAXCALLS);
	CPU::ISR::isr_install_handler(CPU::IDT::SYSCALL_VECTOR, syscall_handler);

	/* Now install all the scheduled system calls: */
	if(syscall_schedule_installs) {
		foreach(packet, syscall_schedule_installs) {
			struct syscall_packet * p = (struct syscall_packet*)packet->value;
			syscall_install_s(p->syscall_name, p->no, p->syscall_addr); /* Install system call! */
		}

		/* Cleanup schedule list: */
		list_destroy(syscall_schedule_installs);
		free(syscall_schedule_installs);
	}
	symbol_remove((char*)"syscall_schedule_install");

	/* Install default system calls: */
	for(uint32_t i = 0; i < num_syscalls; i++)
		syscall_install_n(i, (uintptr_t)syscalls[i]);
	/* Report back how many system calls were installed: */
	kprintf("(# of syscalls: %d) ", syscall_count);
	syscall_initialized = 1; /* All ready */
}

void syscall_install_s(char * syscall_name, int no, uintptr_t syscall_addr) {
	syscall_vector[no] = (syscall_callback_t)syscall_addr;
	hashmap_set(syscall_vector_hash, syscall_name, (void*)no);
	syscall_count++;
}

void syscall_install_s(char * syscall_name, uintptr_t syscall_addr) {
	syscall_install_s(syscall_name, syscall_count, syscall_addr);
}

void syscall_install_n(int no, uintptr_t syscall_addr) {
	char syscall_name[16];
	sprintf(syscall_name, "sysc_%d", syscall_count);
	syscall_install_s(syscall_name, no, syscall_addr);
}

void syscall_install_p(uintptr_t syscall_addr) {
	syscall_install_n(syscall_count, syscall_addr);
}

void syscall_uninstall(uintptr_t syscall_addr) {
	/* Remove address from list: */
	for(int i = 0;i < SYSCALL_MAXCALLS; i++) {
		if(syscall_vector[i] == (syscall_callback_t)syscall_addr) syscall_vector[i] = 0;
		break;
	}

	/* Remove address from hashmap: */
	foreach(key, hashmap_keys(syscall_vector_hash)) {
		syscall_callback_t addr_ptr = (syscall_callback_t)hashmap_get(syscall_vector_hash, key->value);
		if(addr_ptr == (syscall_callback_t)syscall_addr) {
			hashmap_remove(syscall_vector_hash, (void*)addr_ptr);
			break;
		}
	}
	syscall_count--;
}
EXPORT_SYMBOL(syscall_uninstall);

char syscall_schedule_install(char * syscall_name, int no, uintptr_t syscall_addr) {
	if(syscall_initialized) return -1;

	if(!syscall_schedule_installs) {
		/* Initialize system call install scheduler */
		syscall_schedule_installs = list_create();
	}

	/* Create new syscall packet and add it to the schedule list: */
	struct syscall_packet * new_packet = new struct syscall_packet;
	new_packet->syscall_name = syscall_name;
	new_packet->no = no;
	new_packet->syscall_addr = syscall_addr;
	list_insert(syscall_schedule_installs, (void*)new_packet);

	return 0;
}
EXPORT_SYMBOL(syscall_schedule_install);

void syscall_run_n(int intno) {
	intno++;
	asm volatile("mov %0, %%eax; int $0x7F" : :"m"(intno));
}
EXPORT_SYMBOL(syscall_run_n);

void syscall_run_s(char * syscall_name) {
	if(hashmap_has(syscall_vector_hash, syscall_name)) {
		int intno = (int)hashmap_get(syscall_vector_hash, syscall_name) + 1;
		asm volatile("mov %0, %%eax; int $0x7F" : :"m"(intno));
	}
}
EXPORT_SYMBOL(syscall_run_s);
/************************************************/

}
}

/***************************************************/
/********** System Call Default Functions **********/
/***************************************************/
SYSDECL(sys_exit, int retval) {

	return 0;
}

SYSDECL(sys_open, const char * file, int flags, int mode) {

	return 0;
}

SYSDECL(sys_read, int fd, char * ptr, int len) {

	return 0;
}

SYSDECL(sys_write, int fd, char * ptr, int len) {

	return 0;
}

SYSDECL(sys_close, int fd) {

	return 0;
}

SYSDECL(sys_gettimeofday, time_t * tv, void * tz) {

	return 0;
}

SYSDECL(sys_execve, const char * filename, char *const argv[], char *const envp[]) {

	return 0;
}

SYSDECL(sys_fork, void) {

	return 0;
}

SYSDECL(sys_getpid, void) {

	return 0;
}

SYSDECL(sys_sbrk, int size) {

	return 0;
}

SYSDECL(sys_uname, utsname_t * name) {

	return 0;
}

SYSDECL(sys_openpty, int * master, int * slave, char * name, void * _ign0, void * size) {

	return 0;
}

SYSDECL(sys_seek, int fd, int offset, int whence) {

	return 0;
}

SYSDECL(sys_stat, int fd, uintptr_t st) {

	return 0;
}

SYSDECL(sys_mkpipe, void) {

	return 0;
}

SYSDECL(sys_dup2, int old, int _new) {

	return 0;
}

SYSDECL(sys_getuid, void) {

	return 0;
}

SYSDECL(sys_setuid, unsigned int new_uid) {

	return 0;
}

SYSDECL(sys_reboot, void) {

	return 0;
}

SYSDECL(sys_readdir, int fd, int index, struct dirent * entry) {

	return 0;
}

SYSDECL(sys_chdir, char * newdir) {

	return 0;
}

SYSDECL(sys_getcwd, char * buf, size_t size) {

	return 0;
}

SYSDECL(sys_clone, uintptr_t new_stack, uintptr_t thread_func, uintptr_t arg) {

	return 0;
}

SYSDECL(sys_sethostname, char * new_hostname) {

	return 0;
}

SYSDECL(sys_gethostname, char * buffer) {

	return 0;
}

SYSDECL(sys_mkdir, char * path, uint32_t mode) {

	return 0;
}

SYSDECL(sys_shm_obtain, char * path, size_t * size) {

	return 0;
}

SYSDECL(sys_shm_release, char * path) {

	return 0;
}

SYSDECL(sys_kill, signed int process, uint32_t signal) {

	return 0;
}

SYSDECL(sys_signal, uint32_t signum, uintptr_t handler) {

	return 0;
}

SYSDECL(sys_gettid, void) {

	return 0;
}

SYSDECL(sys_yield, void) {

	return 0;
}

SYSDECL(sys_sysfunc, int fn, char ** args) {

	return 0;
}

SYSDECL(sys_sleepabs, unsigned long seconds, unsigned long subseconds) {

	return 0;
}

SYSDECL(sys_sleep, unsigned long seconds, unsigned long subseconds) {

	return 0;
}

SYSDECL(sys_ioctl, int fd, int request, void * argp) {

	return 0;
}

SYSDECL(sys_access, const char * file, int flags) {

	return 0;
}

SYSDECL(sys_statf, char * file, uintptr_t st) {

	return 0;
}

SYSDECL(sys_chmod, char * file, int mode) {

	return 0;
}

SYSDECL(sys_umask, int mode) {

	return 0;
}

SYSDECL(sys_unlink, char * file) {

	return 0;
}

SYSDECL(sys_waitpid, int pid, int * status, int options) {

	return 0;
}

SYSDECL(sys_pipe, int pipes[2]) {

	return 0;
}

SYSDECL(sys_mount, char * arg, char * mountpoint, char * type, unsigned long flags, void * data) {

	return 0;
}

SYSDECL(sys_symlink, char * target, char * name) {

	return 0;
}

SYSDECL(sys_readlink, const char * file, char * ptr, int len) {

	return 0;
}

SYSDECL(sys_lstat, char * file, uintptr_t st) {

	return 0;
}
/***************************************************/
