# Tools, tools' path and flags
TOOLCH = toolchain
LINKER = linker.ld
CXX = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-g++.exe
CC = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-gcc.exe
AS = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-as.exe
NAS = $(TOOLCH)\Tools\NASM\nasm.exe
CFLAGS = -T $(TOOLCH)\$(LINKER) -Itoolchain\Tools\Cross\i686-elf\lib\gcc\i686-elf\4.8.2\include -Isrc -O -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -lgcc -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions

ASFLAGS = 
NASFLAGS = -g -f elf
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
include src\memory\subdir.mk
include src\task\subdir.mk

############### Main targets ###############

all: kernel-link

# Link all those subdir.mk object files into the whole Kernel:
kernel-link: $(OBJS)
	@echo '----------'
	@echo '>>>> Linking Kernel <<<<'
	@echo '>>>> Invoking: Cross i686-elf GCC Linker <<<<'
	$(CXX) $(CFLAGS) -o $(DISKPATH)\$(KOUT) $(OBJS)
	@echo '>>>> Finished building target: $@ <<<<'
	@echo '----------'

clean:
	rm $(BOUT)/*.o
