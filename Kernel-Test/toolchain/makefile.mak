# Tools, tools' path and flags
TOOLCH = toolchain
LINKER = linker.ld
CXX = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-g++.exe
CC = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-gcc.exe
AS = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-as.exe
NAS = $(TOOLCH)\Tools\NASM\nasm.exe
CFLAGS = -T $(TOOLCH)\$(LINKER) -nostartfiles -nostdlib -Wall -Wno-unknown-pragmas

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
