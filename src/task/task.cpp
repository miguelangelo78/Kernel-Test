/*
 * process.cpp
 *
 *      Author: Miguel
 */
#include <system.h>
#include <module.h>
#include <libc/tree.h>

/* Tasking reference:
 * http://wiki.osdev.org/Kernel_Multitasking
 * https://github.com/dthain/basekernel/blob/master/src/process.h
 * https://github.com/dthain/basekernel/blob/master/src/process.c 
 */

namespace Kernel {
namespace Task {

/************************************************/
/************ Tasking Variables *****************/
/************************************************/
char is_tasking = 0;
char is_tasking_initialized = 0;
volatile task_t * current_task;
task_t * main_task;
/************************************************/

/************************************************************/
/******* Tasking/Process prototype functions / externs ******/
/************************************************************/
extern task_t * fetch_next_task(void);
extern void task_removefromtree(task_t * task);
extern void initialize_process_tree();
/************************************************************/

/*************************************/
/******** Task error handling ********/
/*************************************/
void task_error_handle(task_t * task, char error_type) {
	switch(error_type) {
	case 0: /* Couldn't fetch task from queue */ break;
	}
}
/*************************************/

/**********************************/
/********* Task switching *********/
/**********************************/

/**
 * Decide whether the task can live or not (TTL = Time to Live):
 * (Return: 0 - MAY live | 1 - May NOT live)
 **/
inline char task_ttl_canlive(task_t * next_task) {
	if(next_task->pid != 0) {
		/* Decide the switch via PWM conditions: */
		if(next_task->ttl_pwm_mode) {
			if(next_task->ttl++ < next_task->ttl_start)
				return 1; /* This task is not allowed to live while its TTL is counting */
			else
				if(next_task->ttl >= next_task->ttl_fscale)
					next_task->ttl = 0; /* Restart TTL counter AND switch */
		} else { /* Decide the switch via non-PWM conditions */
			if(next_task->ttl < next_task->ttl_fscale) {
				next_task->ttl++;
				return 1; /* This task is not allowed to live while its TTL is counting */
			}
			else
				next_task->ttl = next_task->ttl_start; /* Restart TTL counter AND switch */
		}
	}
	return 0;
}

/*
* Switch to the next ready task.
*
* This is called from the interrupt handler
* for the interval timer to perform standard task switching
*/
void switch_task(status_t new_process_state) {
	if (!is_tasking || !current_task) return; /* Tasking is not yet installed (or it's disabled). */
	IRQ_OFF();

	/* Save registers first: */
	uintptr_t eip = Kernel::CPU::read_eip();
	if(eip == 0x10000) return;
	current_task->regs->eip = eip;
	asm volatile("mov %%esp, %0" : "=r" (current_task->regs->esp)); /* Save ESP */
	asm volatile("mov %%ebp, %0" : "=r" (current_task->regs->ebp)); /* Save EBP */

	/* Update current task state to new state: */
	current_task->status = new_process_state;
	CPU::TSS::tss_set_kernel_stack(current_task->regs->esp);

	/* Reinsert current task into task queue: */
	make_task_ready((task_t*)current_task);

	/* Fetch next task: */
	while(1) {
		task_t * next_task = fetch_next_task();
		if(next_task) {
			if(task_ttl_canlive(next_task))
				continue; /* The task was not allowed to live for this context switch cycle */
			/* Fetch task now: */
			current_task = next_task;
			break;
		} else {
			/* Uh oh, couldn't fetch next task, handle error here */
			task_error_handle(next_task, 0);
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
		: : "r" (current_task->regs->eip), "r" (current_task->regs->esp), "r" (current_task->regs->ebp), "r" (current_task->regs->cr3)
		: "%ebx", "%esp", "%eax"
	);
}

/* PIT Callback: */
unsigned long * timer_ticks;
unsigned long * timer_subticks;

void pit_switch_task(void) {
	wakeup_sleepers(*timer_ticks, *timer_subticks);
	switch_task(TASKST_READY);
}
/**********************************/

/**************************/
/****** Task killing ******/
/**************************/
char irq_already_off = 0;

/* Task reap: Remove task from the tree,
 * which INCLUDES freeing the task itself */
void task_reap(task_t * task) {
	free(task->name);
	task_removefromtree(task);
}

/* Task exit: Cleanup all allocated resources by the task,
 * but DO NOT free the task itself */
void task_exit(int retval) {
	if(!current_task->pid)
		return; /* Do not let the main task kill itself */
	if(!irq_already_off)
		IRQ_OFF();

	/* Cleanup the task: */
	task_free((task_t*)current_task, retval);

	task_t * parent = task_get_parent((task_t*)current_task);
	if(parent)
		wakeup_queue(parent->wait_queue);

	irq_already_off = 0;
	IRQ_RES(); /* Resume switching */
}

/* Every task that returns will end up here: */
void task_return_grave(int retval) {
	IRQ_OFF(); /* Prevent switch context as soon as possible, so we don't lose 'current_task's address */
	irq_already_off = 1; /* This prevents IRQ_OFF to run twice */
	task_exit(retval); /* Commit suicide, or rather, become a zombie */
	for(;;); /* Never return. Remember that this 'for' won't be actually running, we just don't want to run 'ret' */
	KERNEL_FULL_STOP(); /* Just because */
}
/**************************/

/**********************/
/**** Task control ****/
/**********************/
void tasking_enable(char enable) {
	IRQ_OFF();
	is_tasking = enable ? 1 : 0;
	IRQ_RES();
}

/* Duty cycle is expressed from 0% to 100% */
void task_set_ttl(task_t * task, int duty_cycle_or_preload) {
	IRQ_OFF();
	if(task){
		if(task->ttl_pwm_mode) {
			task->ttl_start = task->ttl_fscale - ((duty_cycle_or_preload * task->ttl_fscale) / 100); /* Duty cycle */
			task->ttl = 0;
		} else {
			task->ttl_start = task->ttl = ((duty_cycle_or_preload * task->ttl_fscale) / 100); /* Preload value */
		}
	}
	IRQ_RES();
}

/* Duty cycle is expressed from 0% to 100% */
void task_set_ttl(int pid, int duty_cycle_or_preload) {
	IRQ_OFF();
	task_t * task = task_from_pid(pid);
	if(task)
		task_set_ttl(task, duty_cycle_or_preload);
	IRQ_RES();
}

/* Set TTL frequency scale: */
void task_set_ttl_fscale(task_t * task, int fscale) {
	IRQ_OFF();
	if(task) {
		task->ttl_fscale = fscale <= 0 ? MAX_TTL : (fscale >= MAX_TTL ? MAX_TTL : fscale);
		task_set_ttl(task, 100); /* Set default duty cycle to 100% */
	}
	IRQ_RES();
}

/* Set TTL frequency scale: */
void task_set_ttl_fscale(int pid, int fscale) {
	IRQ_OFF();
	task_t * task = task_from_pid(pid);
	if(task)
		task_set_ttl_fscale(task, fscale);
	IRQ_RES();
}

/* Set TTL mode:
 * (mode - 0: Single pulse burst | 1: PWM) */
void task_set_ttl_mode(task_t * task, char pwm_or_pulse_mode) {
	IRQ_OFF();
	if(task) {
		if(pwm_or_pulse_mode) {
			task->ttl_pwm_mode = 1;
			task->ttl = task->ttl_start = 0;
		} else {
			task->ttl_pwm_mode = 0;
			task->ttl = task->ttl_start = task->ttl_fscale;
		}
	}
	IRQ_RES();
}

/* Set TTL mode:
 * (mode - 0: Single pulse burst | 1: PWM) */
void task_set_ttl_mode(int pid, char pwm_or_pulse_mode) {
	IRQ_OFF();
	task_t * task = task_from_pid(pid);
	if(task)
		task_set_ttl_mode(task, pwm_or_pulse_mode);
	IRQ_RES();
}
/**********************/

/**********************************/
/****** Tasking initializers ******/
/**********************************/
int ctr1 = 0, ctr2 = 0;
static void task1(void) {
	for(;;) {
		IRQ_OFF();
		Point p = Kernel::term.go_to(50, 0);
		Kernel::term.printf("TASK1 (pulse) %d      ", ctr1++);
		Kernel::term.go_to(p.X, p.Y);
		IRQ_RES();
	}
}

static void task2(void) {
	for(;;) {
		IRQ_OFF();
		Point p = Kernel::term.go_to(50, 1);
		Kernel::term.printf("TASK2 (pwm) %d      ", ctr2++);
		Kernel::term.go_to(p.X, p.Y);
		IRQ_RES();
	}
}

extern void task_allocate_stack(task_t * task, uintptr_t stack_size);

void tasking_install(void) {
	IRQ_OFF();

	/* Install the pit callback, which is acting as a callback service: */
	MOD_IOCTL("pit_driver", 1, (uintptr_t)"pit_switch_task", (uintptr_t)pit_switch_task);

	/* Initialize module pointers: */
	timer_ticks    = (unsigned long*)symbol_find("timer_ticks");
	timer_subticks = (unsigned long*)symbol_find("timer_subticks");

	initialize_process_tree();

	/* Initialize the very first task, which is the main thread that was already running: */
	current_task = main_task = spawn_rootproc();

	tasking_enable(1); /* Allow tasking to work */
	is_tasking_initialized = 1;

	IRQ_RES(); /* Kickstart tasking */

	/* Test tasking: */
	task_t * t1 = spawn_childproc(main_task);
	task_allocate_stack(t1, 0x8000);
	set_task_environment(t1, task1, main_task->regs->eflags, main_task->regs->cr3);
	make_task_ready(t1);

	task_t * t2 = spawn_childproc(main_task);
	task_allocate_stack(t2, 0x8000);
	set_task_environment(t2, task2, main_task->regs->eflags, main_task->regs->cr3);
	make_task_ready(t2);
}
/**********************************/

}
}
