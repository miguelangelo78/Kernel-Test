.section .symbols

.extern __bss
.type __bss, @function

.extern __code
.type __code, @function

.extern __data
.type __data, @function

.extern __end
.type __end, @function

.extern _bss
.type _bss, @function

.extern _code
.type _code, @function

.extern _data
.type _data, @function

.extern _end
.type _end, @function

.extern _isr0
.type _isr0, @function

.extern _isr1
.type _isr1, @function

.extern _isr10
.type _isr10, @function

.extern _isr11
.type _isr11, @function

.extern _isr12
.type _isr12, @function

.extern _isr127
.type _isr127, @function

.extern _isr13
.type _isr13, @function

.extern _isr14
.type _isr14, @function

.extern _isr15
.type _isr15, @function

.extern _isr16
.type _isr16, @function

.extern _isr17
.type _isr17, @function

.extern _isr18
.type _isr18, @function

.extern _isr19
.type _isr19, @function

.extern _isr2
.type _isr2, @function

.extern _isr20
.type _isr20, @function

.extern _isr21
.type _isr21, @function

.extern _isr22
.type _isr22, @function

.extern _isr23
.type _isr23, @function

.extern _isr24
.type _isr24, @function

.extern _isr25
.type _isr25, @function

.extern _isr26
.type _isr26, @function

.extern _isr27
.type _isr27, @function

.extern _isr28
.type _isr28, @function

.extern _isr29
.type _isr29, @function

.extern _isr3
.type _isr3, @function

.extern _isr30
.type _isr30, @function

.extern _isr31
.type _isr31, @function

.extern _isr4
.type _isr4, @function

.extern _isr5
.type _isr5, @function

.extern _isr6
.type _isr6, @function

.extern _isr7
.type _isr7, @function

.extern _isr8
.type _isr8, @function

.extern _isr9
.type _isr9, @function

.extern _Z6memsetPvim
.type _Z6memsetPvim, @function

.extern _ZN6Kernel3CPU3GDT8gdt_initEv
.type _ZN6Kernel3CPU3GDT8gdt_initEv, @function

.extern _ZN6Kernel3CPU3IDT12idt_set_gateEhPFvvEth
.type _ZN6Kernel3CPU3IDT12idt_set_gateEhPFvvEth, @function

.extern _ZN6Kernel3CPU3IDT4initEv
.type _ZN6Kernel3CPU3IDT4initEv, @function

.extern _ZN6Kernel3CPU3IDT9idt_flushEv
.type _ZN6Kernel3CPU3IDT9idt_flushEv, @function

.extern _ZN6Kernel3CPU3ISR12isrs_installEv
.type _ZN6Kernel3CPU3ISR12isrs_installEv, @function

.extern _ZN6Kernel3CPU3ISR13fault_handlerEPNS0_6regs_tE
.type _ZN6Kernel3CPU3ISR13fault_handlerEPNS0_6regs_tE, @function

.extern _ZN6Kernel4termE
.type _ZN6Kernel4termE, @function

.extern _ZN6Kernel5kmainEPNS_4Init11multiboot_tEjj
.type _ZN6Kernel5kmainEPNS_4Init11multiboot_tEjj, @function

.extern _ZN7Console12reset_cursorEv
.type _ZN7Console12reset_cursorEv, @function

.extern _ZN7Console4fillEc
.type _ZN7Console4fillEc, @function

.extern _ZN7Console4initEv
.type _ZN7Console4initEv, @function

.extern _ZN7Console4putcEcc
.type _ZN7Console4putcEcc, @function

.extern _ZN7Console4putsEPcc
.type _ZN7Console4putsEPcc, @function

.extern _ZN7Console5clearEv
.type _ZN7Console5clearEv, @function

.extern _ZN7ConsoleC1Ev
.type _ZN7ConsoleC1Ev, @function

.extern _ZN7ConsoleC2Ev
.type _ZN7ConsoleC2Ev, @function

.extern bss
.type bss, @function

.extern code
.type code, @function

.extern data
.type data, @function

.extern end
.type end, @function

.extern kernel_symbols_end
.type kernel_symbols_end, @function

.extern kernel_symbols_start
.type kernel_symbols_start, @function

.extern mboot
.type mboot, @function

.extern start
.type start, @function

.global kernel_symbols_start
kernel_symbols_start:

.long __bss
.asciz "__bss"

.long __code
.asciz "__code"

.long __data
.asciz "__data"

.long __end
.asciz "__end"

.long _bss
.asciz "_bss"

.long _code
.asciz "_code"

.long _data
.asciz "_data"

.long _end
.asciz "_end"

.long _isr0
.asciz "_isr0"

.long _isr1
.asciz "_isr1"

.long _isr10
.asciz "_isr10"

.long _isr11
.asciz "_isr11"

.long _isr12
.asciz "_isr12"

.long _isr127
.asciz "_isr127"

.long _isr13
.asciz "_isr13"

.long _isr14
.asciz "_isr14"

.long _isr15
.asciz "_isr15"

.long _isr16
.asciz "_isr16"

.long _isr17
.asciz "_isr17"

.long _isr18
.asciz "_isr18"

.long _isr19
.asciz "_isr19"

.long _isr2
.asciz "_isr2"

.long _isr20
.asciz "_isr20"

.long _isr21
.asciz "_isr21"

.long _isr22
.asciz "_isr22"

.long _isr23
.asciz "_isr23"

.long _isr24
.asciz "_isr24"

.long _isr25
.asciz "_isr25"

.long _isr26
.asciz "_isr26"

.long _isr27
.asciz "_isr27"

.long _isr28
.asciz "_isr28"

.long _isr29
.asciz "_isr29"

.long _isr3
.asciz "_isr3"

.long _isr30
.asciz "_isr30"

.long _isr31
.asciz "_isr31"

.long _isr4
.asciz "_isr4"

.long _isr5
.asciz "_isr5"

.long _isr6
.asciz "_isr6"

.long _isr7
.asciz "_isr7"

.long _isr8
.asciz "_isr8"

.long _isr9
.asciz "_isr9"

.long _Z6memsetPvim
.asciz "_Z6memsetPvim"

.long _ZN6Kernel3CPU3GDT8gdt_initEv
.asciz "_ZN6Kernel3CPU3GDT8gdt_initEv"

.long _ZN6Kernel3CPU3IDT12idt_set_gateEhPFvvEth
.asciz "_ZN6Kernel3CPU3IDT12idt_set_gateEhPFvvEth"

.long _ZN6Kernel3CPU3IDT4initEv
.asciz "_ZN6Kernel3CPU3IDT4initEv"

.long _ZN6Kernel3CPU3IDT9idt_flushEv
.asciz "_ZN6Kernel3CPU3IDT9idt_flushEv"

.long _ZN6Kernel3CPU3ISR12isrs_installEv
.asciz "_ZN6Kernel3CPU3ISR12isrs_installEv"

.long _ZN6Kernel3CPU3ISR13fault_handlerEPNS0_6regs_tE
.asciz "_ZN6Kernel3CPU3ISR13fault_handlerEPNS0_6regs_tE"

.long _ZN6Kernel4termE
.asciz "_ZN6Kernel4termE"

.long _ZN6Kernel5kmainEPNS_4Init11multiboot_tEjj
.asciz "_ZN6Kernel5kmainEPNS_4Init11multiboot_tEjj"

.long _ZN7Console12reset_cursorEv
.asciz "_ZN7Console12reset_cursorEv"

.long _ZN7Console4fillEc
.asciz "_ZN7Console4fillEc"

.long _ZN7Console4initEv
.asciz "_ZN7Console4initEv"

.long _ZN7Console4putcEcc
.asciz "_ZN7Console4putcEcc"

.long _ZN7Console4putsEPcc
.asciz "_ZN7Console4putsEPcc"

.long _ZN7Console5clearEv
.asciz "_ZN7Console5clearEv"

.long _ZN7ConsoleC1Ev
.asciz "_ZN7ConsoleC1Ev"

.long _ZN7ConsoleC2Ev
.asciz "_ZN7ConsoleC2Ev"

.long bss
.asciz "bss"

.long code
.asciz "code"

.long data
.asciz "data"

.long end
.asciz "end"

.long kernel_symbols_end
.asciz "kernel_symbols_end"

.long kernel_symbols_start
.asciz "kernel_symbols_start"

.long mboot
.asciz "mboot"

.long start
.asciz "start"

.global kernel_symbols_end
kernel_symbols_end:
