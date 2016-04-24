
#include <system.h>
#include <module.h>

namespace Kernel {
namespace Task {

volatile int * current_process = 0;

/*
* Switch to the next ready task.
*
* This is called from the interrupt handler for the interval timer to
* perform standard task switching.
*/
void switch_task(void) {
	if (!current_process) return; /* Tasking is not yet installed. */
}

void task1(void) {
	for(;;);
}

void task2(void) {
	for(;;);
}

void tasking_install(void) {
	MOD_IOCTL("pit_driver", 1, (uintptr_t)"switch_task", (uintptr_t)switch_task);

	task1();
}

}
}
