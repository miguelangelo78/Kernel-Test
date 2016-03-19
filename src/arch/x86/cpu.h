/*
 * cpu.h
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#ifndef SRC_ARCH_X86_CPU_H_
#define SRC_ARCH_X86_CPU_H_

#include <libc.h>

namespace Kernel {
namespace CPU {
	enum REGLIST {
		gs,fs,es,ds,edi,esi,ebp,esp,ebx,edx,ecx,eax,eip,cs,eflags,usersp,ss
	};

	typedef struct {
		unsigned int gs, fs, es, ds;
		unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
		unsigned int int_no, err_code;
		unsigned int eip, cs, eflags, useresp, ss;
	} regs_t;

	inline unsigned int read_reg(enum REGLIST regid) {
		unsigned int reg = 0;
		switch(regid) {
		case esp: asm volatile("mov %%esp, %%eax\nmov %%eax, %0\n":"=r"(reg)::"%ebx"); break;
		case ebp: asm volatile("mov %%ebp, %%eax\nmov %%eax, %0\n":"=r"(reg)::"%ebx"); break;
		default: break;
		}
		return reg;
	}
}
}

#endif /* SRC_ARCH_X86_CPU_H_ */
