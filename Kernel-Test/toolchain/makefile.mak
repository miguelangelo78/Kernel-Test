# Tools, tools' path and flags
TOOLCH = toolchain
SRCPATH = src
LINKER = linker.ld
CXX = $(TOOLCH)\Tools\Cross\i686-elf\bin\i686-elf-g++.exe
CFLAGS = -T  $(TOOLCH)\$(LINKER) -nostartfiles -nostdlib -Wall -Wno-unknown-pragmas

# Output constants (filenames and paths)
DISKPATH = iso
BOUT = build
# Kernel filename:
KOUT = ksharp.bin
STAGE2OBJ = $(BOUT)\ksharp_stage2.o

################# Includes #################

include src\subdir.mk

############### Main targets ###############

all: kernel-link

# Link all those subdir.mk object files into the whole Kernel:
kernel-link: $(OBJS)
	@echo '----------'
	@echo '>>>> Linking Kernel <<<<'
	@echo '>>>> Invoking: Cross i686-elf GCC Linker <<<<'
	$(CXX) $(CFLAGS) -o $(DISKPATH)\$(KOUT) $(STAGE2OBJ) $(OBJS)
	@echo '>>>> Finished building target: $@ <<<<'
	@echo '----------'

clean:
	rm $(BOUT)/*.o