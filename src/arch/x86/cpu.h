/*
 * cpu.h
 *
 *  Created on: 19/03/2016
 *      Author: Miguel
 */

#ifndef SRC_ARCH_X86_CPU_H_
#define SRC_ARCH_X86_CPU_H_

#include <libc.h>
#include <attr.h>

namespace Kernel {
namespace CPU {
	enum REGLIST {
		gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, eip, cs, eflags, usersp, ss
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
		case eflags: asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(reg)::"%eax"); break;
		default: break;
		}
		return reg;
	}

	/*
	 * asm-CPUID result structure.
	 */
	typedef struct cpuid_type {
	    uint32_t eax, ebx, ecx, edx;
	} cpuid_t;

	/*
	 * Table of available Processor descriptions.
	 * (Source: Intel/AMD online manuals.)
	 */
	typedef struct cpu_ident {
	    unsigned int family;
	    unsigned int model;
	    char         manufacturerid[13];
	    char         expln[64];
	} cpu_identifier;

	enum cpuid_requests {
	  CPUID_GETVENDORSTRING,
	  CPUID_GETFEATURES,
	  CPUID_GETTLB,
	  CPUID_GETSERIAL,

	  CPUID_INTELEXTENDED = 0x80000000,
	  CPUID_INTELFEATURES,
	  CPUID_INTELBRANDSTRING,
	  CPUID_INTELBRANDSTRINGMORE,
	  CPUID_INTELBRANDSTRINGEND,
	};

	static cpu_identifier cpu_identifier_map[] =
	{
	    /* Family, Model, Manufacturer, Explanation */
	    { 4, 0, "GenuineIntel", "Intel 486 DX" },
	    { 4, 1, "GenuineIntel", "Intel 486 DX" },
	    { 4, 2, "GenuineIntel", "Intel 486 SX" },
	    { 4, 3, "GenuineIntel", "Intel 487, 486 DX2 or DX2 OverDrive" },
	    { 4, 4, "GenuineIntel", "Intel 486 SL" },
	    { 4, 5, "GenuineIntel", "Intel 486 SX2" },
	    { 4, 7, "GenuineIntel", "Intel 486 DX2 w/ writeback" },
	    { 4, 8, "GenuineIntel", "Intel 486 DX4 or DX4 OverDrive" },
	    { 5, 1, "GenuineIntel", "Intel Pentium 60/66" },
	    { 5, 2, "GenuineIntel", "Intel Pentium 75/90/100/120/133/150/166/200" },
	    { 5, 3, "GenuineIntel", "Intel Pentium OverDrive for 486 Systems" },
	    { 5, 4, "GenuineIntel", "Intel Pentium w/ MMX 166/200/233" },
	    { 6, 1, "GenuineIntel", "Intel Pentium Pro" },
	    { 6, 3, "GenuineIntel", "Intel Pentium II" },
	    { 6, 5, "GenuineIntel", "Future Intel P6" },
	    { 5, 0, "AuthenticAMD", "AMD-K5-PR75/90/100" },
	    { 5, 1, "AuthenticAMD", "AMD-K5-PR120/133" },
	    { 5, 2, "AuthenticAMD", "AMD-K5-PR150/166" },
	    { 5, 3, "AuthenticAMD", "AMD-K5-PR200" },
	    { 5, 6, "AuthenticAMD", "AMD-K6 Model 6 (2.9 / 3.2 V Types)" },
	    { 5, 7, "AuthenticAMD", "AMD-K6 Model 7 (2.2 V Types)" },
	    { 5, 8, "AuthenticAMD", "AMD-K6-2 3D Model 8" },
	    { 5, 9, "AuthenticAMD", "AMD-K6-2 3D+ Model 9" },
	    { 0, 0, "", ""} /* End marker, don't remove! */
	};

	/*
	 * CPU features as returned by CPUID 1
	 * Intel (CPUID 1) and AMD (CPUID 0x80000001) maps differ slightly.
	 */
	typedef struct cpu_ftr {
	    unsigned int bit;
	    char         name[8];
	    char         expln[64];
	} cpu_feature;

	/* This is according to Intel's manuals. */
	static cpu_feature intel_cpu_feature_map[] =
	{   {  0,   "FPU", "FPU on-chip" },
	    {  1,   "VME", "Virtual-8086 Mode Enhancement" },
	    {  2,    "DE", "Debugging Extensions" },
	    {  3,   "PSE", "Page Size Extensions" },
	    {  4,   "TSC", "Time Stamp Counter" },
	    {  5,   "MSR", "Model Specific Registers with RDMSR/WRMSR Support" },
	    {  6,   "PAE", "Physical Adress Extensions" },
	    {  7,   "MCE", "Machine Check Exception" },
	    {  8,   "CXS", "CMPXCHG8B Instruction Supported" },
	    {  9,   "APIC", "On-chip APIC" },
	    { 11,   "SEP", "Fast System Call" },
	    { 12,   "MTRR", "Memory Type Range Registers" },
	    { 13,   "PGE", "PTE Global Bit" },
	    { 14,   "MCA", "Machine Check Architecture" },
	    { 15,   "CMOV", "Conditional Move/Cmp. Inst." },
	    { 16,  "FGPAT", "Page Attribute Table / CMOVcc" },
	    { 17, "PSE-36", "36-bit Page Size Extension" },
	    { 18,    "PN", "Processor Serial Number (enabled)" },
	    { 23,   "MMX", "MMX Extension" },
	    { 24,   "FXSR", "Fast FP/MMX Save and Restore" },
	    { 25,   "XMM", "Streaming SIMD Extensions" },
	    {255,    "", "" } /* End marker, don't remove! */
	};

	/* These are AMD specific features detected with CPUID 0x80000001. */
	static cpu_feature amd_cpu_feature_map[] =
	{   {  0,  "FPU", "FPU on-chip" },
	    {  1,  "VME", "Virtual-8086 Mode Enhancement" },
	    {  2,   "DE", "Debugging Extensions" },
	    {  3,  "PSE", "Page Size Extensions" },
	    {  4,  "TSC", "Time Stamp Counter" },
	    {  5,  "MSR", "Model Specific Registers with RDMSR/WRMSR Support" },
	    {  7,  "MCE", "Machine Check Exception" },
	    {  8,  "CXS", "CMPXCHG8B Instruction Supported" },
	    { 10,  "SEP", "Fast System Call (pobably A-stepping processor)" },
	    { 11,  "SEP", "Fast System Call (B or later stepping)" },
	    { 13,  "PGE", "PTE Global Bit" },
	    { 15, "CMOV", "Conditional Move/Cmp. Inst." },
	    { 23,  "MMX", "MMX Extension" },
	    { 31,  "3DN", "3D Now Extension" },
	    {255,    "", "" } /* End marker, don't remove! */
	};

	/*
	 * This table describes the possible cache and TLB configurations
	 * as documented by Intel. For now AMD doesn't use this but gives
	 * exact cache layout data on CPUID 0x8000000x.
	 *
	 * MAX_CACHE_FEATURES_ITERATIONS limits the possible cache information
	 * to 80 bytes (of which 16 bytes are used in generic Pentii2).
	 * With 80 possible caches we are on the safe side for one or two years.
	 *
	 * Strange enough no BHT, BTB or return stack data is given this way...
	 */
	#define MAX_CACHE_FEATURES_ITERATIONS 8

	typedef struct cache_ftr {
	    unsigned char label;
	    char          expln[80];
	} cache_feature;

	static cache_feature intel_cache_feature_map[] =
	{   { 0x01 , "Instruction TLB for 4K-Byte Pages: 32 entries, 4-way set associative" },
	    { 0x02 , "Instruction TLB for 4M-Byte Pages:  2 entries, fully associative" },
	    { 0x03 , "Data      TLB for 4K-Byte Pages: 64 entries, 4-way set associative" },
	    { 0x04 , "Data      TLB for 4M-Byte Pages:  8 entries, 4-way set associative" },
	    { 0x06 , "Instruction Cache:    8K Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x08 , "Instruction Cache:   16K Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x0a , "Data Cache:   8K Bytes, 32 Byte lines, 2-way set associative" },
	    { 0x0c , "Data Cache:   16K Bytes, 32 Byte lines, 2-way or 4-way set associative" },
	    { 0x40 , "No L2 Cache" },
	    { 0x41 , "Unified L2 Cache:  128K Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x42 , "Unified L2 Cache:  256K Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x43 , "Unified L2 Cache:  512K Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x44 , "Unified L2 Cache: 1M Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x45 , "Unified L2 Cache: 2M Bytes, 32 Byte lines, 4-way set associative" },
	    { 0x00 , "NULL" }
	};

	/*
	 * cpuid calls the processor's internal CPUID instruction and returns the
	 *     16-byte result. The type of CPUID to return is determined by the
	 *     argument number. No range check is performed on number as all
	 *     tested processors return 0s on illegal CPUID input values.
	 *     However a call to CPUID 0 gives the maximum reasonable value to
	 *     call CPUID with, so the caller may check this.
	 *
	 */
	static inline cpuid_t cpuid(unsigned int number) {
		cpuid_t result; /* structure to return */

		/* Store number in %eax, call CPUID, save %eax, %ebx, %ecx, %edx in variables.
		   As output constraint "m=" has been used as this keeps gcc's optimizer
		   from overwriting %eax, %ebx, %ecx, %edx by accident */
		asm("movl %4, %%eax; cpuid; movl %%eax, %0; movl %%ebx, %1; movl %%ecx, %2; movl %%edx, %3;"
			: "=a" (result.eax),
			  "=b" (result.ebx),
			  "=c" (result.ecx),
			  "=d" (result.edx) /* output */
			  : "a"  (number)   /* input */
			  : "eax", "ebx", "ecx", "edx" /* no changed registers except output registers */
		);
		return result;
	}

	static char vendor[32] = { 0 };

	static inline char* cpu_vendor() {
		asm volatile("xor %%eax, %%eax; cpuid; movl %%ebx, %0; movl %%edx, 4+%0; movl %%ecx, 8+%0":"=m"(vendor));
		return vendor;
	}

	#define cpu_is_intel(cpuid_struct) (cpuid_struct.ebx == 0x756e6547)	/* Intel Magic code */
	#define cpu_is_amd(cpuid_struct)  (cpuid_struct.ebx == 0x68747541) /* AMD Magic code */
	#define cpu_is_unknown(cpuid_struct) (cpu_is_intel(cpuid_struct) | cpu_is_amd(cpuid_struct))

	/************* X86 specific Definitions: *************/

	#define X86_SEGMENT_SELECTOR( seg, rpl )  (((seg)<<3)+(rpl))

	#define X86_SEGMENT_KERNEL_CODE  X86_SEGMENT_SELECTOR(1,0)
	#define X86_SEGMENT_KERNEL_DATA  X86_SEGMENT_SELECTOR(2,0)
	#define X86_SEGMENT_USER_CODE    X86_SEGMENT_SELECTOR(3,3)
	#define X86_SEGMENT_USER_DATA    X86_SEGMENT_SELECTOR(4,3)
	#define X86_SEGMENT_TSS          X86_SEGMENT_SELECTOR(5,0)

	typedef struct {
		int32_t	eax;
		int32_t	ebx;
		int32_t	ecx;
		int32_t	edx;
		int32_t	esi;
		int32_t	edi;
		int32_t	ebp;
	} __packed x86_regs_t;

	typedef struct {
		unsigned carry:1;
		unsigned reserved0:1;
		unsigned parity:1;
		unsigned reserved1:1;

		unsigned auxcarry:1;
		unsigned reserved2:1;
		unsigned zero:1;
		unsigned sign:1;

		unsigned trap:1;
		unsigned interrupt:1;
		unsigned direction:1;
		unsigned overflow:1;

		unsigned iopl:2;
		unsigned nested:1;
		unsigned reserved3:1;

		unsigned resume:1;
		unsigned v86:1;
		unsigned align:1;
		unsigned vinterrupt:1;

		unsigned vpending:1;
		unsigned id:1;
	} __packed x86_eflags_t;

	typedef struct {
		x86_regs_t regs2;
		int32_t	old_ebp;
		int32_t	old_addr;
		x86_regs_t regs1;
		int32_t	ds;
		int32_t	intr_num;
		int32_t	intr_code;
		int32_t	eip;
		int32_t	cs;
		x86_eflags_t eflags;
		int32_t	esp;
		int32_t	ss;
	} __packed x86_stack_t;
}
}

#endif /* SRC_ARCH_X86_CPU_H_ */
