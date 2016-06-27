# Kernel-Test
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;An experimental project for training and testing with a Kernel for x86 systems (and maybe more) using C++ language. There will be no OS as this is focused specifically on the Kernel domain. We will however add userspace and some test programs.

![Kernel screenshot](http://i.imgur.com/UNDVGDN.png)
![Kernel screenshot 2](http://i.imgur.com/PQ29Gcw.png)
![Kernel screenshot 3](http://i.imgur.com/FxDpD1g.png)

# How To Install
This project can be compiled and executed in both Windows and Linux distros.

Simply Download (save as...) and Run the following scripts:
* [**Windows**](https://raw.githubusercontent.com/miguelangelo78/Kernel-Test/master/toolchain/Windows/kerneldev_toolkit_downloader.bat)
* [**Linux - Soon**](#)

Follow all the prompts that are given during installation.

# How to Build and Run

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Access the folder 'toolchain/[Windows | Debian]/' and there you'll find 9 scripts:  

	1- **bindump**: Reads the kernel file with 'readelf' and opens a text editor with the result  
	2- **bootable_usb**: Makes your flash drive bootable with the kernel  
	3- **build**: Build the Kernel  
	4- **build_and_run**: Build the Kernel and run it immediately  
	5- **clean**: Delete all object files from the folder build  
	6- **rebuild**: Generates automatically the makefiles for all source files and rebuilds the Kernel  
	7- **rebuild_and_run**: Generates automatically the makefiles for all source files, rebuilds the Kernel and runs it  
	8/9- **launch_i386 / launch_x86_64**: Simply launch the Kernel on QEMU/VirtualBox. (**NOTE**: If you want to launch on VirtualBox you'll have to install it and set up the VM and point the .iso path to VBox for boot up. The same applies to Bochs).  

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;For a more productive way of development, simply use Eclipse IDE (with CDT plugin) and import this repository into the workspace. The scripts are already set up and ready to be built and run.

# How to Run on real machines
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;If you want a more realistic approach to Kernel Development, you can simply access the folder 'toolchain/Windows' and run the script bootable_usb.bat. It'll ask what drive letter you want to install the kernel to, and you should select the Pendrive's correct letter. Then, simply boot the pendrive on any machine and it should work.
These scripts were prepared for only Windows for now. Linux will come soon.

# x86 References
1 - FlingOS Project (http://www.flingos.co.uk/, https://www.youtube.com/watch?v=_xlO9MawAqY)

2 - OSDev Wiki (http://wiki.osdev.org/Main_Page)

3 - OSDev Forum (http://forum.osdev.org/)

4 - Bran's Kernel Development (http://www.osdever.net/bkerndev/Docs/title.htm)

5 - BrokenThorn Entertainment - OS Development Series (http://www.brokenthorn.com/Resources/OSDevIndex.html)

6 - Bona Fide - OS Development Tutorials (http://www.osdever.net/tutorials/)

7 - Intel® 64 and IA-32 Architectures Software Developer’s Manual (http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-manual-325462.pdf)

8 - Syslinux Wiki (http://www.syslinux.org/wiki/index.php/SYSLINUX)

9 - OSDev Brasil - (http://www.osdevbrasil.net/)

# x86 Virtual Machines

1 - QEMU (Windows - http://qemu.weilnetz.de/)

2 - VirtualBox (https://www.virtualbox.org/)

> 2.1 - Virtualbox Debug reference (https://www.virtualbox.org/manual/ch12.html)

3 - Bochs (http://bochs.sourceforge.net/)

4 - VMware (http://www.vmware.com/)

# Project structure
>1) Documentation - The Documentation of this Kernel will progressively go here

>2) build - All objects file will go here

>2.1) build/modules - All built modules will go here

>3) iso - **bootloader** (ISOLinux) files and **bootable HDD disk** (.iso), including **binary kernel**
	
>4) src - **Entire source code** of the Project
	
>5) toolchain/* - Tools for building (**nasm** + cross **compiler** (uncommitted)), for **ISO generation**, **Launchers** and **generators** (aka Utility scripts, e.g. makefile generator and versioning script)

# Source code structure


> - **Kernel**

	1. Init  
		1.1. kmain.cpp (Start point)  
		1.2. kinit.h (Multiboot header)  
		1.3. initrd.cpp/.h (Allows modules to be loaded)  

	2. Arch
		2.1. x86
			2.1.1. GDT (Global Descriptor Table)  
			2.1.2. IDT (Interrupt Descriptor Table)  
			2.1.3. ISR (Interrupt Service Routine)  
			2.1.4. IRQ (Interrupt Request)  
			2.1.5. Bootloader Stage 2 (asm)  
			2.1.7. cpu.h - CPU constants and register definitions  
			2.1.6. VM8086.cpp - Virtual 8086 Mode  

	3. Memory
		3.1. kmemory.cpp (Physical Memory Manager)
		3.2. alloc.cpp (Slab allocator (malloc) by Kevin Lange (https://github.com/klange))

	4. IO
		4.1. io.h (Input Output from/to the CPU)
		4.2. terminal.cpp/.h (Text mode screen)
		4.3. serial.cpp/.h (Serial COM port class (mainly for debugging))
		 
	5. Error
		5.1. error.cpp (Panic handler aka BSOD)
	
	6. Debug
		6.1. log.h (Error/Info/Warning logger)
	
	7. VFS
		7.1. vfs.cpp / fs.h (Virtual File System. initrd "bypasses" this system only at startup)
		  
	8. Exec
	 	8.1. elf.cpp/.h (ELF object parser, loader and executor)

	9. Task
		9.1. process.cpp
		9.2. spin.cpp
		9.3. tss_flush.s

	10. Kernel Information
		10.1. version.cpp/.h - Kernel constants and versioning system

	11. Video
		11.1. video.cpp/.h - Video Manager/Installer/Initializer for the Kernel

> - **Modules**

	 1. module.cpp/.h (Installs/Uninstalls modules and Function/Variable Symbols)
	 2. pit.cpp - Programmable Interval Timer
	 3. cmos.cpp - BIOS information
	 4. keyboard.cpp / kbd_scan.h - Keyboard driver
	 5. video_vesa.cpp - VBE graphics driver
	 6. fpu.cpp - Floating Point Unit driver
	 
> - **Libraries**

	 1. libc.cpp/.h (Essential C functions and macros)
	 2. kprintf.cpp (Another essential C functions file)
	 2. attr.h (Attribute macros)
	 2. bit.h (Bit handler macros)
	 3. stdint.h (Declaration of types)
	 3. va_list.h (Variable arguments macros)
	 4. limits.h (Variable size limits macros)
	 5. hashmap.cpp/.h (Hashmap data structure)
	 6. list.cpp/.h (Linked list data structure)
	 7. tree.cpp/.h (Tree data structure)

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
