# Tools, tools' path and flags
TOOLCH = toolchain
LINKER = linker.ld
CXX = $(TOOLCH)\Tools\Windows\Cross\i686-elf\bin\i686-elf-g++.exe
CC = $(TOOLCH)\Tools\Windows\Cross\i686-elf\bin\i686-elf-gcc.exe
CXX_LLVM = clang++
CC_LLVM = clang
AS = $(TOOLCH)\Tools\Windows\Cross\i686-elf\bin\i686-elf-as.exe
NAS = $(TOOLCH)\Tools\Windows\NASM\nasm.exe
CPPFLAGS = -T $(TOOLCH)\$(LINKER) -Itoolchain\Tools\Windows\Cross\i686-elf\lib\gcc\i686-elf\4.8.2\include -Isrc -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -lgcc -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions
LLVMCPPFLAGS = -ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions -IC:\llvm\lib\clang\3.8.0\include -Isrc
CFLAGS = -T $(TOOLCH)\$(LINKER) -Itoolchain\Tools\Windows\Cross\i686-elf\lib\gcc\i686-elf\4.8.2\include -Isrc -g -ffreestanding -fbuiltin -Wall -Wextra
LLVMCFLAGS = -ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -fno-exceptions -IC:\llvm\lib\clang\3.8.0\include -Isrc
ASFLAGS = 
NASFLAGS = -g -f elf32

# Output constants (filenames and paths)
DISKPATH = iso
BOUT = build
# Kernel filename:
KOUT = ksharp.bin
STAGE2OBJ = $(BOUT)\ksharp_stage2.o

################# Includes #################

include src\subdir.mk
include src\arch\x86\subdir.mk
include src\arch\x86\gdt\subdir.mk
include src\arch\x86\idt\subdir.mk
include src\arch\x86\irq\subdir.mk
include src\arch\x86\isr\subdir.mk
include src\libc\subdir.mk
include src\libc\data_struct\subdir.mk
include src\memory\subdir.mk
include src\mod\subdir.mk
include src\task\subdir.mk

############### Main targets ###############

all: kernel-link

# Link all those subdir.mk object files into the whole Kernel:
kernel-link: $(OBJS) $(MODS)
	@echo '----------'
	@echo 'Toolchain: LLVM Toolchain'
	@echo '>>>> Linking Kernel <<<<'
	@echo '>>>> Invoking: Cross i686-elf GCC Linker <<<<'
	$(CXX) $(CPPFLAGS) -o $(DISKPATH)\$(KOUT) $(OBJS)
	@echo '>>>> Finished building target: $@ <<<<'
	@echo '----------'

clean:
	rm $(BOUT)/*.o
	rm $(BOUT)/modules/*.mod
