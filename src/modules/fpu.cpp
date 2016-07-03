/*
 * fpu.cpp
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>

/**
 * Set the FPU control word
 *
 * @param cw What to set the control word to.
 */
void set_fpu_cw(const uint16_t cw) {
	asm volatile("fldcw %0" :: "m"(cw));
}

static void disable_fpu(void) {
	size_t temp;
	asm volatile ("mov %%cr0, %0" : "=r"(temp));
	temp |= 1 << 3;
	asm volatile ("mov %0, %%cr0" :: "r"(temp));
}

static void enable_fpu(void) {
	asm volatile("clts");
	size_t temp;
	asm volatile("mov %%cr4, %0" : "=r"(temp));
	temp |= 3 << 9;
	asm volatile("mov %0, %%cr4" :: "r"(temp));
}

uint8_t saves[512] __attribute__((aligned(16)));

static void restore_fpu(void) {

}

static void save_fpu(void) {

}

static void switch_fpu(void) {
	disable_fpu();
}

static void fpu_inv_op(Kernel::CPU::regs_t * regs) {
	enable_fpu();
	// TODO
	kprintf("\n!!FPU HANDLER!!\n");
}

static void init_fpu(void) {
	asm volatile("fninit");
	set_fpu_cw(0x37F);
}

static int fpu_mod_init(void) {
	SYA(isr_install_handler, Kernel::CPU::IDT::ISR_INVOPCODE, fpu_inv_op);
	SYA(isr_install_handler, Kernel::CPU::IDT::ISR_DEVICEUN,  fpu_inv_op);
	return 0;
}

static int fpu_mod_finit(void) {
	return 0;
}

static uintptr_t fpu_ioctl(void * ioctl_packet) {

	return 0;
}

MODULE_DEF(fpu_driver, fpu_mod_init, fpu_mod_finit, MODT_SYS, "Miguel S.", fpu_ioctl);
