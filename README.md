# Kernel-Test
An experimental project for training and testing with a Kernel for x86 systems (and maybe more) using C++ language. There will be no OS as this is focused specifically on the Kernel domain. We will however add userspace and some test programs.

![Kernel screenshot](http://i.imgur.com/Y1FauPJ.png)

# x86 References
1 - FlingOS Project (http://www.flingos.co.uk/, https://www.youtube.com/watch?v=_xlO9MawAqY)

2 - OSDev Wiki (http://wiki.osdev.org/Main_Page)

3 - OSDev Forum (http://forum.osdev.org/)

4 - Bran's Kernel Development (http://www.osdever.net/bkerndev/Docs/title.htm)

5 - BrokenThorn Entertainment - OS Development Series (http://www.brokenthorn.com/Resources/OSDevIndex.html)

6 - Bona Fide - OS Development Tutorials (http://www.osdever.net/tutorials/)

7 - Intel® 64 and IA-32 Architectures Software Developer’s Manual (http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-manual-325462.pdf)

8 - Syslinux Wiki (http://www.syslinux.org/wiki/index.php/SYSLINUX)

# x86 Virtual Machines

1 - QEMU (Windows - http://qemu.weilnetz.de/)

2 - VirtualBox (https://www.virtualbox.org/)

> 2.1 - Virtualbox Debug reference (https://www.virtualbox.org/manual/ch12.html)

3 - Bochs (http://bochs.sourceforge.net/)

3 - VMware (http://www.vmware.com/)

# Project structure

>1) iso - **bootloader** (ISOLinux) files and **bootable HDD disk** (.iso), including **binary kernel**
	
>2) src - **Entire source code** of the Project
	
>3) toolchain - Tools for building (**nasm** + cross **compiler** (uncommitted)) and for **ISO generation**
	
>4) scripts - **Launchers** and **generators** (aka Utility scritps, e.g. makefile generator)

# Source code structure

 

> - **Kernel**
	 1. **Init**
		 4. kmain.cpp (Start point)
		 5. kinit.h (Multiboot header)
	 2. **Arch**
		 2. x86
			 2. GDT (Global Descriptor Table)
			 3. IDT (Interrupt Descriptor Table)
			 4. ISR (Interrupt Service Routine)
			 5. IRQ (Interrupt Request)
			 6. **Bootloader Stage 2** (asm)
	 3. **Memory**
		 3. kmemory.cpp (Physical Memory Manager)
	 4. **IO**
		 5. io.h (Input Output from/to the CPU)
		 6. terminal.cpp/.h (Text mode screen)
	 5. **Error**
		 5. error.cpp (Panic handler aka BSOD)
	 6. **Debug**
		 7. log.h (Error/Info/Warning logger)
		 
> - **Modules**
	 1. **module**.h (Installs/Uninstalls modules and Function/Variable Symbols)

> - **Libraries**
	 1. **libc**.cpp/.h (Essential C functions and macros)
	 2. **kprintf.cpp** (Another essential C functions file)
	 2. **attr**.h (Attribute macros)
	 2. **bit**.h (Bit handler macros)
	 3. **stdint**.h (Declaration of types)
	 3. **va_list**.h (Variable arguments macros)
	 4. **limits**.h (Variable size limits macros)

**Important element:** system.h 
**Description:** glue header for including all kernel features, such as:

 1. Memory segment selectors
 2. Segment pointers
 3. Initial ESP register value
 4. Important utility macros (assert, asm, volatile, kernel pause/full pause/full stop)
 5. Registers structure
 5. Text mode terminal
 7. GDT (init only)
 7. IDT (init and vector list only)
 8. ISR (install only)
 8. IRQ (init, install/uninstall, interrupt and resume only)
 9. Memory
	 10. Manager (physical and virtual (kmalloc), also paging)
	 11. Allocator (slab allocator (malloc))
 12. Error (panic handler)
