/*
 * usermode.cpp
 *
 *  Created on: 01/07/2016
 *      Author: Miguel
 */
#include <system.h>
#include <module.h>

extern "C" { void usermode_enter_asm(uintptr_t location, uintptr_t stack); }

namespace Kernel {
namespace CPU {

void usermode_enter(uintptr_t location, int argc, char ** argv, uintptr_t stack) {
	kprintf("(jump addr: 0x%x stack: 0x%x)", location, stack);
	if(location) {
		/* Prepare user mode (e.g.: stack and jump location): */
		IRQ_OFF();
		CPU::TSS::tss_set_kernel_stack(Task::current_task->image.stack);

		PUSH(stack, uintptr_t, (uintptr_t)argv);
		PUSH(stack, int, argc);

		kprintfc(COLOR_WARNING, "\n* Kernel is now in usermode *");

		/* Complete operation using assembly: */
		usermode_enter_asm(location, stack);
		/* We will not return here. The kernel should now be executing the Operating System at this point */
	} else {
		/* Jump location provided is not valid. We cannot jump */
		kprintfc(COLOR_BAD, "\n!ERROR: Jump Address is null!");
	}
    kprintf(" ");
}

}
}
