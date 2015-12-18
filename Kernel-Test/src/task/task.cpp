#include <system.h>
#include <process.h>

/*
* Switch to the next ready task.
*
* This is called from the interrupt handler for the interval timer to
* perform standard task switching.
*/
void switch_task(uint8_t reschedule) {
	/* Tasking is not yet installed. */
	if (!current_process) return;
}