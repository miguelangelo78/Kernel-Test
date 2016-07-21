# Tools, tools' path and flags
TOOLCH = toolchain
LINKER = linker.ld
CXX = $(TOOLCH)/Windows/Tools/Cross/i686-elf/bin/i686-elf-g++
CC = $(TOOLCH)/Windows/Tools/Cross/i686-elf/bin/i686-elf-gcc
LD = $(TOOLCH)/Windows/Tools/Cross/i686-elf/bin/i686-elf-ld
CXX_LLVM = clang++
CC_LLVM = clang
AS = $(TOOLCH)/Windows/Tools/Cross/i686-elf/bin/i686-elf-as
NAS = $(TOOLCH)/Windows/Tools/NASM/nasm
CPPFLAGS = -T $(TOOLCH)/$(LINKER) -Isrc -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions
LLVMCPPFLAGS = -ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions -IC:/llvm/lib/clang/3.8.0/include -Isrc
CFLAGS = -T $(TOOLCH)/$(LINKER) -Isrc -g -finline-functions -ffreestanding -fbuiltin -Wall -Wextra
LLVMCFLAGS = -ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -fno-exceptions -IC:/llvm/lib/clang/3.8.0/include -Isrc
ASFLAGS = 
NASFLAGS = -g -f elf32
CPPFLAGS_MODS = -Isrc -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -lgcc -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions

# Output constants (filenames and paths)
DISKPATH = iso
BOUT = obj
# Kernel filename:
KOUT = ksharp.bin
################# Includes #################

include src/subdir.mk
include src/arch/x86/subdir.mk
include src/arch/x86/gdt/subdir.mk
include src/arch/x86/idt/subdir.mk
include src/arch/x86/irq/subdir.mk
include src/arch/x86/isr/subdir.mk
include src/kernel_headers/subdir.mk
include src/libc/subdir.mk
include src/libc/data_struct/subdir.mk
include src/memory/subdir.mk
include src/modules/subdir.mk
include src/modules/clock/subdir.mk
include src/modules/fs/subdir.mk
include src/modules/kbd/subdir.mk
include src/modules/mouse/subdir.mk
include src/modules/pci/subdir.mk
include src/ring3/subdir.mk
include src/task/subdir.mk
include src/terminal/subdir.mk
include src/version/subdir.mk

############### Main targets ###############

all: kernel-link

# Link all those subdir.mk object files into the whole Kernel:
kernel-link: $(OBJS) $(MODS)
	@echo '----------'
	@echo 'Toolchain: LLVM Toolchain'
	@echo '>>>> Linking Kernel <<<<'
	@echo '>>>> Invoking: LLVM C++ Linker <<<<'
	$(CXX) $(CPPFLAGS) -o $(DISKPATH)\$(KOUT) $(OBJS)
	@echo '>>>> Finished building target: $@ <<<<'
	@echo '----------'

clean:
	rm $(BOUT)/*.o
	rm $(BOUT)/modules/*.mod
