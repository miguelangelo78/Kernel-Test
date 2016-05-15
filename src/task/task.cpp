
#include <system.h>
#include <module.h>
#include <libc/tree.h>

/* Tasking reference:
 * http://wiki.osdev.org/Kernel_Multitasking
 * https://github.com/dthain/basekernel/blob/master/src/process.h
 * https://github.com/dthain/basekernel/blob/master/src/process.c */

namespace Kernel {
namespace Task {

/* Max number of processes the kernel will switch: */
#define MAX_PID 32768
#define MAX_TTL 60000 /* Maximum allowable time to live (expressed in switching count, not by ms or seconds) */

#define TASK_STACK_SIZE (PAGE_SIZE * 2)

char is_tasking = 0;
char is_tasking_initialized = 0;
task_t * current_task;
task_t * main_task;
tree_t * tasktree;
list_t * tasklist;
uint16_t next_pid = 0;

/****************************** Task tree getters and setters ******************************/
int fetch_index = 0;
int tasklist_size = 0;

task_t * fetch_next_task(void) {
	if(fetch_index > tasklist_size)
		fetch_index = 0;
	return (task_t*)list_get(tasklist, fetch_index++)->value;
}

void task_addtotree(task_t * parent_task, task_t * new_task) {
	/* Add task to tree: */
	//parent_task->next = new_task;

	/* Insert into switch queue: */
	list_insert(tasklist, new_task);
	tasklist_size++;
}

/****************************** Task switching ******************************/
/*
* Switch to the next ready task.
*
* This is called from the interrupt handler for the interval timer to
* perform standard task switching.
*/
void switch_task(char new_process_state) {
	if (!is_tasking || !current_task) return; /* Tasking is not yet installed (or it's disabled). */
	IRQ_OFF();

	/* Save registers first: */
	uintptr_t eip = Kernel::CPU::read_eip();
	if(eip == 0x10000) return;
	current_task->regs.eip = eip;
	asm volatile("mov %%esp, %0" : "=r" (current_task->regs.esp)); /* Save ESP */
	asm volatile("mov %%ebp, %0" : "=r" (current_task->regs.ebp)); /* Save EBP */

	/* Update current task state to new state: */
	current_task->state = new_process_state;

	/* Fetch next task: */
	while(1) {
		task_t * next_task = fetch_next_task();
		if(next_task) {
			if(next_task->ttl < MAX_TTL && next_task->pid != 0) {
				next_task->ttl++;
				continue; /* This task is not allowed to live while its TTL is counting */
			} else {
				next_task->ttl = next_task->start_ttl; /* Restart TTL counter */
			}
			current_task = next_task;
			break;
		} else {
			/* Uh oh, couldn't fetch next task, handle error here */
			return;
		}
	}

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

/* PIT Callback: */
void pit_switch_task(void) {
	switch_task(TASKST_READY);
}
/***************************************************************************/

/****************************** Task killing ******************************/
char irq_already_off = 0;

void task_free(task_t * task_to_free) {
	/* Clean up stack: */
	free((void*)task_to_free->regs.esp);
}

void task_kill(int pid) {
	if(!pid) return; /* Do not let the main task kill itself */
	if(!irq_already_off)
		IRQ_OFF();

	kprintf("\nKILLED PID: %d\n", pid);

	/* Remove the task from the list, then cleanup the rest (remove process from tree, deallocate task's stack and more) */
	task_t * task_to_kill = (task_t*)list_get(tasklist, pid)->value; /* Store it first so we can clean up everything */
	list_remove(tasklist, pid);
	tasklist_size--;

	/* Remove from tree: */

	/* Cleanup everything else: */
	task_free(task_to_kill);

	irq_already_off = 0;
	IRQ_RES(); /* Resume switching */
}

/* Every task that returns will end up here: */
void task_return_grave(void) {
	IRQ_OFF(); /* Prevent switch context as soon as possible, so we don't lose 'current_task's address */
	irq_already_off = 1; /* This prevents IRQ_OFF to run twice */
	task_kill(current_task->pid); /* Commit suicide */
	for(;;); /* Never return. Remember that this 'for' won't be actually running, we just don't want to run 'ret' */
}
/***************************************************************************/

/****************************** Task creation ******************************/
task_t * task_create(char * task_name, void (*entry)(void), uint32_t eflags, uint32_t pagedir) {
	IRQ_OFF();

	task_t * task = new task_t;

	task->regs.eflags = eflags;
	task->regs.cr3 = pagedir;
	task->state = TASKST_CRADLE;
	task->name = task_name;
	task->regs.eax = task->regs.ebx = task->regs.ecx = task->regs.edx = task->regs.esi = task->regs.edi = 0;
	task->next = 0;
	task->ttl = task->start_ttl = MAX_TTL;

	if(entry) {
		task->regs.eip = (uint32_t) entry;
		task->regs.esp = (uint32_t) malloc(TASK_STACK_SIZE) + TASK_STACK_SIZE;

		/* Set next pid: */
		task->pid = ++next_pid;

		/* Prepare X86 stack: */
		uint32_t * stack_ptr = (uint32_t*)(task->regs.esp);
		/* Parse it and configure it: */
		Kernel::CPU::x86_stack_t * stack = (Kernel::CPU::x86_stack_t*)stack_ptr;
		stack->regs2.ebp = (uint32_t)(stack_ptr + 28);
		stack->old_ebp = (uint32_t)(stack_ptr + 32);
		stack->old_addr = (unsigned)task_return_grave;
		stack->ds = X86_SEGMENT_USER_DATA;
		stack->cs = X86_SEGMENT_USER_CODE;
		stack->eip = task->regs.eip;
		stack->eflags.interrupt = 1;
		stack->eflags.iopl = 3;
		stack->esp = task->regs.esp;
		stack->ss = X86_SEGMENT_USER_DATA;
		stack->regs2.eax = (uint32_t)task_return_grave; /* Return address of a task */
	} else {
		if(!is_tasking_initialized) {
			/* If entry is null, then we're allocating the very first process, which is the main core task */
			task->pid = next_pid;
		} /* else we ignore it, we don't want to run a normal task with an entry point of address 0! */
	}

	IRQ_RES();
	return task;
}

task_t * task_create_and_run(char * task_name, task_t * parent_task, void (*entry)(void), uint32_t eflags, uint32_t pagedir) {
	/* Create task: */
	task_t * new_task = task_create(task_name, entry, eflags, pagedir);
	IRQ_OFF(); /* Keep IRQs off */

	/* Add it to the tree (and by adding, the switcher function will now switch to this process): */
	task_addtotree(parent_task, new_task);

	IRQ_RES();
	return new_task;
}

task_t * task_create_and_run(char * task_name, void (*entry)(void), uint32_t eflags, uint32_t pagedir) {
	return task_create_and_run(task_name, current_task, entry, eflags, pagedir);
}
/***************************************************************************/

/****************************** Task control ******************************/
void tasking_enable(char enable) {
	IRQ_OFF();
	is_tasking = enable ? 1 : 0;
	IRQ_RES();
}

void task_set_ttl(int pid, int ttl) {
	IRQ_OFF();
	task_t * task = (task_t*)list_get(tasklist, pid)->value;
	if(task) task->start_ttl = task->ttl = ttl;
	IRQ_RES();
}
/***************************************************************************/

int ctr1 = 0, ctr2 = 0;
static void task1(void) {
	for(;;)
		Kernel::serial.printf("TASK1 %d\n", ctr1++);
}

static void task2(void) {
	for(;;)
		Kernel::serial.printf("TASK2 %d\n", ctr2++);
}

/****************************** Task initializers ******************************/
void tasking_install(void) {
	IRQ_OFF();

	/* Install the pit callback, which is acting as a callback service: */
	MOD_IOCTL("pit_driver", 1, (uintptr_t)"pit_switch_task", (uintptr_t)pit_switch_task);

	/* Initialize the very first task, which is the main thread that was already running: */
	current_task = main_task =
			task_create((char*)"rootproc", 0, Kernel::CPU::read_reg(Kernel::CPU::eflags), (uint32_t)Kernel::Memory::Man::curr_dir->table_entries);

	/* Initialize task list and tree: */
	tasklist = list_create();
	tasktree = tree_create();
	tree_set_root(tasktree, main_task);
	list_insert(tasklist, main_task);

	tasking_enable(1); /* Allow tasking to work */
	is_tasking_initialized = 1;
	IRQ_RES(); /* Kickstart tasking */

	/* Test tasking: */
	task_t * t1 = task_create_and_run((char*)"task1", task1, current_task->regs.eflags, current_task->regs.cr3);
	task_t * t2 = task_create_and_run((char*)"task2", task2, current_task->regs.eflags, current_task->regs.cr3);
}
/***************************************************************************/

}
}
