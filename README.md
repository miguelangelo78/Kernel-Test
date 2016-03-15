# Kernel-Test
An experimental project for training and testing with a Kernel for x86 systems (and maybe more) using C++ language. There will be no OS as this is focused specifically on the Kernel domain. I will however add userspace and some test programs.

![Kernel screenshot](http://i.imgur.com/pccSWrE.png)

# Before Building and Running
You'll be required to make small preparations before compiling and testing the kernel.

**Steps:**

**Step 1)** Download this repository, and put it wherever you want (Desktop maybe?)

 **Step 2)**
 
**For Windows**

> Download the prebuilt cross compiler i686-elf for Windows from here: [i686-elf Windows Cross Compiler Download](https://docs.google.com/uc?export=download&confirm=o2Dx&id=0B85K_c7mx3QjUnZuaFRPWlBIcXM).
> Extract it and rename the folder to i686-elf.

**For Linux**

> (Soon...)

**Step 3)** Create a new folder inside toolchain/Tools/{Windows|Linux} (Linux soon...) named 'Cross', and put the previous folder (i686-elf) inside the Cross folder. For example, you'd have the path: toolchain/Tools/Windows/Cross/i686-elf/ ......

**Step 4)**

**For Windows**

> Install Prebuilt Clang from here: [Clang Download](http://llvm.org/releases/download.html)

**For Linux**

> (Soon...)


**Step 5)**

**For Windows**
> Download QEMU binaries for windows from here: [QEMU Download](http://qemu.weilnetz.de/) and install them normally on your computer

**For Linux**
> (Soon...)

# How to Build and Run
**For Windows**

> Access the folder 'toolchain/Windows/' and there you'll find 5 batch scripts:
	> 1- **build.bat**: Build the Kernel and run it immediately
	> 2- **rebuild.bat**: Generates automatically the makefiles for all source files, rebuilds the Kernel and runs it
	> 3 - **clean.bat**: Delete all object files from the folder build
	> 4 - **launch_i386.bat / launch_x86_64.bat**: Simply launch the Kernel on QEMU/VirtualBox. (**NOTE**: If you want to launch on VirtualBox you'll have to install it and set up the VM and point the .iso path to VBox for boot up. The same applies to Bochs).

For a more productive way of development, simply use Eclipse IDE (with CDT plugin) and import this repository into the workspace. The scripts are already set up and ready to be built and run.

**For Linux**
> (Soon...)

# How to Run on real machines
If you want a more realistic approach to Kernel Development, you can simply access the folder 'iso' and run the script bootable_usb.bat. It'll ask what drive letter you want to install the kernel to, and you should select the Pendrive's correct letter. Then, simply boot the pendrive on any machine and it should work.
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
