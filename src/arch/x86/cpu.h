/*
 * cpu.h
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#ifndef SRC_ARCH_X86_CPU_H_
#define SRC_ARCH_X86_CPU_H_


namespace Kernel {
namespace CPU {
	typedef struct {
		unsigned int gs, fs, es, ds;
		unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
		unsigned int int_no, err_code;
		unsigned int eip, cs, eflags, useresp, ss;
	} regs_t;
}
}

#endif /* SRC_ARCH_X86_CPU_H_ */
