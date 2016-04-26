
#include <system.h>
#include <module.h>

/* Tasking reference:
 * http://wiki.osdev.org/Kernel_Multitasking
 * https://github.com/dthain/basekernel/blob/master/src/process.h
 * https://github.com/dthain/basekernel/blob/master/src/process.c */

namespace Kernel {
namespace Task {

task_t * current_task;
task_t * main_task;
char is_tasking = 0;

/*
* Switch to the next ready task.
*
* This is called from the interrupt handler for the interval timer to
* perform standard task switching.
*/
extern "C" { uintptr_t read_eip(void); }

void switch_task(char new_process_state) {
	if (!is_tasking || !current_task) return; /* Tasking is not yet installed (or it's disabled). */
	IRQ_OFF();

	Kernel::serial.printf("SWITCH\n");

	/* Save registers first: */
	uintptr_t eip = read_eip();
	if(eip == 0x10000) return;
	current_task->regs.eip = eip;
	asm volatile("mov %%esp, %0" : "=r" (current_task->regs.esp));
	asm volatile("mov %%ebp, %0" : "=r" (current_task->regs.ebp));

	/* Update current task state: */
	current_task->state = new_process_state;
	/* Fetch next task: */
	current_task = current_task->next;

	/* Acknowledge PIT interrupt: */
	Kernel::CPU::IRQ::irq_ack(Kernel::CPU::IRQ::IRQ_PIT);

	/* Restore new process registers and jump/continue the task: */
	asm volatile (
			"mov %0, %%ebx\n"
			"mov %1, %%esp\n"
			"mov %2, %%ebp\n"
			"mov %3, %%cr3\n"
			"mov $0x10000, %%eax\n" /* read_eip() will return 0x10000 */
			"sti\n" /* Enable interrupts again */
			"jmp *%%ebx" /* Jump! */
			: : "r" (current_task->regs.eip), "r" (current_task->regs.esp), "r" (current_task->regs.ebp), "r" (current_task->regs.cr3)
			: "%ebx", "%esp", "%eax");
}

void task_kill(void) {
	kprintf("KILLED MYSELF");
}

/* Every task that returns will end up here: */
void task_died(void) {
	task_kill(); /* Commit suicide */
	for(;;); /* Never return. Remember that this 'for' won't be actually running, we just don't want to run 'ret' */
}

task_t * task_create(void (*entry)(void), uint32_t eflags, uint32_t pagedir) {
	IRQ_OFF();

	task_t * task = new task_t;

	task->regs.eflags = eflags;
	task->regs.cr3 = pagedir;
	task->state = TASKST_CRADLE;
	task->next = 0;

	if(entry) {
		task->regs.eax = task->regs.ebx = task->regs.ecx = task->regs.edx = task->regs.esi = task->regs.edi = 0;
		task->regs.eip = (uint32_t) entry;
		task->regs.esp = (uint32_t) malloc(0x1000) + 0x1000;

		/* Prepare X86 stack: */
		uint32_t * stack_ptr = (uint32_t*)(task->regs.esp);
		/* Parse it and configure it: */
		Kernel::CPU::x86_stack_t * stack = (Kernel::CPU::x86_stack_t*)stack_ptr;
		stack->regs2.ebp = (uint32_t)(stack_ptr + 28);
		stack->old_ebp = (uint32_t)(stack_ptr + 32);
		stack->old_addr = (unsigned)task_died;
		stack->ds = X86_SEGMENT_USER_DATA;
		stack->cs = X86_SEGMENT_USER_CODE;
		stack->eip = task->regs.eip;
		stack->eflags.interrupt = 1;
		stack->eflags.iopl = 3;
		stack->esp = task->regs.esp;
		stack->ss = X86_SEGMENT_USER_DATA;
		stack->regs2.eax = (uint32_t)task_died; /* Return address of a task */
	} else {
		/* If entry is null, then we're allocating the very first process, which is the main core task */
	}

	IRQ_RES();
	return task;
}

void tasking_enable(char enable) {
	IRQ_OFF();
	is_tasking = enable ? 1 : 0;
	IRQ_RES();
}

/* PIT Callback: */
void pit_switch_task(void) {
	switch_task(TASKST_READY);
}

static void task1(void) {
	kprintf("\n\ntask 1 (0x%x): RUN\n\n", current_task->regs.esp);
}

void tasking_install(void) {
	IRQ_OFF();
	MOD_IOCTL("pit_driver", 1, (uintptr_t)"pit_switch_task", (uintptr_t)pit_switch_task);

	main_task = task_create(0, Kernel::CPU::read_reg(Kernel::CPU::eflags), (uint32_t)Kernel::Memory::Man::curr_dir->table_entries);
	current_task = main_task;
	task_t * t = task_create(task1, current_task->regs.eflags, current_task->regs.cr3);

	main_task->next = t;
	t->next = main_task;

	tasking_enable(1);
	IRQ_RES();
}

}
}
