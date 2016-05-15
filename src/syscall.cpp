/*
 * syscall.cpp
 *
 *  Created on: 15 May 2016
 *      Author: miguel
 */
#include <system.h>
#include <libc/hashmap.h>

namespace Kernel {
namespace Syscall {

uint32_t syscall_count = 0;
syscall_callback_t * syscall_vector;
hashmap_t * syscall_vector_hash;

void syscall_handler(CPU::regs_t * regs) {
	regs->eax--;
	if(regs->eax >= SYSCALL_MAXCALLS) return;

	syscall_callback_t cback = syscall_vector[regs->eax];
	if(!cback) return;
	/* Run system call: */
	uint32_t ret = cback(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}

void call(void) {
	kprintf("CALLED!");
}

void syscalls_initialize(void) {
	syscall_vector = new syscall_callback_t[SYSCALL_MAXCALLS];
	memset(syscall_vector, 0, SYSCALL_MAXCALLS);
	syscall_vector_hash = hashmap_create(SYSCALL_MAXCALLS);
	CPU::ISR::isr_install_handler(CPU::IDT::SYSCALL_VECTOR, syscall_handler);
	kprintf("(# of syscalls: %d) ", syscall_count);
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

void syscall_uninstall(syscall_callback_t syscall_addr) {
	/* Remove address from list: */
	for(int i = 0;i < SYSCALL_MAXCALLS; i++) {
		if(syscall_vector[i] == syscall_addr) syscall_vector[i] = 0;
		break;
	}

	/* Remove address from hashmap: */
	foreach(key, hashmap_keys(syscall_vector_hash)) {
		syscall_callback_t addr_ptr = (syscall_callback_t)hashmap_get(syscall_vector_hash, key->value);
		if(addr_ptr == syscall_addr) {
			hashmap_remove(syscall_vector_hash, (void*)addr_ptr);
			break;
		}
	}
	syscall_count--;
}

void syscall_run_n(int intno) {
	intno++;
	asm volatile("mov %0, %%eax; int $0x7F" : :"m"(intno));
}

void syscall_run_s(char * syscall_name) {
	if(hashmap_has(syscall_vector_hash, syscall_name)) {
		int intno = (int)hashmap_get(syscall_vector_hash, syscall_name) + 1;
		asm volatile("mov %0, %%eax; int $0x7F" : :"m"(intno));
	}
}

}
}
