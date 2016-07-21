/*
 * signal.cpp
 *
 *  Created on: 21/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <module.h>
#include <task.h>

namespace Kernel {
namespace Task {

/**********************/
/** SIGNAL VARIABLES **/
/**********************/
static spin_lock_t sig_lock;
static spin_lock_t sig_lock_b;

list_t * rets_from_sig;

char isdeadly[] = {
	0, /* 0? */
	1, /* SIGHUP     */
	1, /* SIGINT     */
	2, /* SIGQUIT    */
	2, /* SIGILL     */
	2, /* SIGTRAP    */
	2, /* SIGABRT    */
	2, /* SIGEMT     */
	2, /* SIGFPE     */
	1, /* SIGKILL    */
	2, /* SIGBUS     */
	2, /* SIGSEGV    */
	2, /* SIGSYS     */
	1, /* SIGPIPE    */
	1, /* SIGALRM    */
	1, /* SIGTERM    */
	1, /* SIGUSR1    */
	1, /* SIGUSR2    */
	0, /* SIGCHLD    */
	0, /* SIGPWR     */
	0, /* SIGWINCH   */
	0, /* SIGURG     */
	0, /* SIGPOLL    */
	3, /* SIGSTOP    */
	3, /* SIGTSTP    */
	0, /* SIGCONT    */
	3, /* SIGTTIN    */
	3, /* SIGTTOUT   */
	1, /* SIGVTALRM  */
	1, /* SIGPROF    */
	2, /* SIGXCPU    */
	2, /* SIGXFSZ    */
	0, /* SIGWAITING */
	1, /* SIGDIAF    */
	0, /* SIGHATE    */
	0, /* SIGWINEVENT*/
	0, /* SIGCAT     */
};
/**********************/

/******************************/
/** SIGNALING IMPLEMENTATION **/
/******************************/
void enter_signal_handler(uintptr_t location, int signum, uintptr_t stack) {
	IRQ_OFF();
	asm volatile(
		"mov %2, %%esp\n"
		"pushl %1\n"           /* Argument count   */
		"pushl $" STRSTR(0xFFFFDEAF) "\n"
		"mov $0x23, %%ax\n"    /* Segment selector */
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"mov %%esp, %%eax\n"   /* Stack -> EAX */
		"pushl $0x23\n"        /* Segment selector again */
		"pushl %%eax\n"
		"pushf\n"              /* Push flags */
		"popl %%eax\n"         /* Fix the Interrupt flag */
		"orl  $0x200, %%eax\n"
		"pushl %%eax\n"
		"pushl $0x1B\n"
		"pushl %0\n"           /* Push the entry point */
		"iret\n"
		: : "m"(location), "m"(signum), "r"(stack) : "%ax", "%esp", "%eax"
	);
}

void handle_signal(task_t * proc, signal_t * sig) {
	uintptr_t handler = sig->handler;
	uintptr_t signum  = sig->signum;
	free(sig);

	if (proc->finished)
		return;

	if (signum == 0 || signum >= NUMSIGNALS)
		return; /* Ignore */

	if (!handler) {
		char dowhat = isdeadly[signum];
		if (dowhat == 1 || dowhat == 2) {
			kexit(128 + signum);
			__builtin_unreachable();
		}
		return;
	}

	if (handler == 1)
		return; /* Ignore */

	uintptr_t stack = 0xFFFF0000;
	if (proc->syscall_regs->useresp < 0x10000100)
		stack = proc->image.user_stack;
	else
		stack = proc->syscall_regs->useresp;

	/* Not marked as ignored, must call signal */
	enter_signal_handler(handler, signum, stack);
}
EXPORT_SYMBOL(handle_signal);

void return_from_signal_handler(void) {
	if (__builtin_expect(!rets_from_sig, 0))
		rets_from_sig = list_create();

	spin_lock(sig_lock);
	list_insert(rets_from_sig, (task_t *)current_task);
	spin_unlock(sig_lock);

	switch_task(0);
}

void fix_signal_stacks(void) {
	uint8_t redo_me = 0;
	if (rets_from_sig) {
		spin_lock(sig_lock_b);
		while (rets_from_sig->head) {
			spin_lock(sig_lock);
			node_t * n = list_dequeue(rets_from_sig);
			spin_unlock(sig_lock);
			if (!n)
				continue;
			task_t * p = (task_t*)n->value;
			free(n);
			if (p == current_task) {
				redo_me = 1;
				continue;
			}
			p->thread.esp = p->signal_state.esp;
			p->thread.eip = p->signal_state.eip;
			p->thread.ebp = p->signal_state.ebp;
			memcpy((void *)(p->image.stack - TASK_STACK_SIZE), p->signal_kstack, TASK_STACK_SIZE);
			free(p->signal_kstack);
			p->signal_kstack = 0;
			make_task_ready(p);
		}
		spin_unlock(sig_lock_b);
	}
	if (redo_me) {
		spin_lock(sig_lock);
		list_insert(rets_from_sig, (task_t*)current_task);
		spin_unlock(sig_lock);
		switch_task(0);
	}
}

int send_signal(pid_t task, uint32_t signal) {
	task_t * receiver = task_from_pid(task);

	if (!receiver)
		return 1; /* Invalid pid */

	if (signal > NUMSIGNALS)
		return 2; /* Invalid signal */

	if (receiver->user != current_task->user && current_task->user != USER_ROOT_UID)
		return 3; /* We will not allow a user to send signals to a different user */

	if (receiver->finished)
		return 4; /* Can't send signals to finished processes */

	if (!receiver->signals.functions[signal] && !isdeadly[signal])
		return 5; /* If we're blocking a signal and it's not going to kill us, don't deliver it */

	/* Append signal to list */
	signal_t * sig = (signal_t*)malloc(sizeof(signal_t));
	sig->handler = (uintptr_t)receiver->signals.functions[signal];
	sig->signum  = signal;
	memset(&sig->registers_before, 0, sizeof(regs_t));

	if (!task_is_ready(receiver))
		make_task_ready(receiver); /* Push the task into the switch queue */
	list_insert(receiver->signal_queue, sig);
	return 0;
}
EXPORT_SYMBOL(send_signal);
/******************************/

}
}
