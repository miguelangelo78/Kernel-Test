/*
 * vm8086.cpp
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#include <system.h>

/* Information:
 * http://wiki.osdev.org/Virtual_8086_Mode,
 * http://wiki.osdev.org/Virtual_Monitor,
 * https://sourceforge.net/p/prettyos/code/HEAD/tree/trunk/Source/kernel/tasking/vm86.c */

namespace Kernel {
namespace VM8086 {

char v86_detect(void) {
	char ret = 0;
	asm volatile("smsw %%ax; and $1, %0" : "=r"(ret) : : "%ax");
	return ret;
}

unsigned v86_peekb(unsigned seg, unsigned off) {

}

unsigned v86_peekw(unsigned seg, unsigned off) {

}

void v86_pokeb(unsigned seg, unsigned off, unsigned val) {

}

void v86_pokew(unsigned seg, unsigned off, unsigned val) {

}

void v86_push16(CPU::regs_t *regs, unsigned value) {

}

void v86_int(CPU::regs_t *regs, unsigned int_num) {

}

void v86_test(void) {

	for(;;);
}


}
}

