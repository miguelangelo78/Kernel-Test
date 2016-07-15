/*
 * fpu.cpp
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>

static task_t * fpu_thread = 0;

static void init_fpu(void); /* Prototype */

/**
 * Set the FPU control word
 *
 * @param cw What to set the control word to.
 */
static void set_fpu_cw(const uint16_t cw) {
	asm volatile("fldcw %0" :: "m"(cw));
}

/**
 * Disable FPU and SSE so it traps to the kernel
 */
static void disable_fpu(void) {
	size_t temp;
	asm volatile ("mov %%cr0, %0" : "=r"(temp));
	temp |= 1 << 3;
	asm volatile ("mov %0, %%cr0" :: "r"(temp));
}

/**
 * Enable the FPU and SSE
 */
static void enable_fpu(void) {
	asm volatile("clts");
	size_t temp;
	asm volatile("mov %%cr4, %0" : "=r"(temp));
	temp |= 3 << 9;
	asm volatile("mov %0, %%cr4" :: "r"(temp));
}

static uint8_t saves[512] __attribute__((aligned(16)));

/**
 * Restore the FPU for a process
 */
static void restore_fpu(task_t * task) {
	memcpy(&saves, (uint8_t *)&task->thread.fp_regs, 512);
	asm volatile ("fxrstor %0" : "=m"(saves));
}

/**
 * Save the FPU for a process
 */
static void save_fpu(task_t * task) {
	asm volatile ("fxsave %0" : "=m"(saves));
	memcpy((uint8_t *)&task->thread.fp_regs, &saves, 512);
}

/**
 * Disable FPU and SSE so it traps to the kernel
 */
static void switch_fpu(void) {
	disable_fpu();
}

/**
 * Kernel trap for FPU usage when FPU is disabled
 */
static void fpu_inv_op(Kernel::CPU::regs_t * regs) {
	enable_fpu();

	task_t * current_task = (task_t*)symbol_find("current_task");
	/* If this is the tread that last used the FPU, do nothing: */
	if(fpu_thread == current_task)
		return;

	if(fpu_thread)
		save_fpu(fpu_thread);

	fpu_thread = (task_t *)current_task;
	if(!fpu_thread->thread.fpu_enabled) {
		/*
		 * If the FPU has not been used in this thread previously,
		 * we need to initialize it.
		 */
		init_fpu();
		fpu_thread->thread.fpu_enabled = 1;
		return;
	}
	/* Otherwise we restore the context for this thread. */
	restore_fpu(fpu_thread);
}

static void init_fpu(void) {
	asm volatile("fninit");
	set_fpu_cw(0x37F);
}

static int fpu_mod_init(void) {
	SYA(isr_install_handler, Kernel::CPU::IDT::ISR_INVOPCODE, fpu_inv_op);
	SYA(isr_install_handler, Kernel::CPU::IDT::ISR_DEVICEUN,  fpu_inv_op);
	symbol_add("switch_fpu", (uintptr_t)switch_fpu);
	return 0;
}

static int fpu_mod_finit(void) {
	return 0;
}

static uintptr_t fpu_ioctl(void * ioctl_packet) {
	return 0;
}

MODULE_DEF(fpu_driver, fpu_mod_init, fpu_mod_finit, MODT_SYS, "Miguel S.", fpu_ioctl);
