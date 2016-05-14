# Tools, tools' path and flags
TOOLCH = toolchain
LINKER = linker.ld
CXX = $(TOOLCH)/Debian/Tools/Cross/i686-elf/bin/i686-elf-g++
CC = $(TOOLCH)/Debian/Tools/Cross/i686-elf/bin/i686-elf-gcc
LD = $(TOOLCH)/Debian/Tools/Cross/i686-elf/bin/i686-elf-ld
CXX_LLVM = clang++-3.8
CC_LLVM = clang-3.8
AS = $(TOOLCH)/Debian/Tools/Cross/i686-elf/bin/i686-elf-as
NAS = $(TOOLCH)/Debian/Tools/NASM/nasm
CPPFLAGS = -T $(TOOLCH)/$(LINKER) -Itoolchain/Debian/Tools/Cross/i686-elf/lib/gcc/i686-elf/include -Isrc -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions
LLVMCPPFLAGS = -ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions -I/usr/lib/llvm-3.8/lib/clang/3.8.0/include -Isrc
CFLAGS = -T $(TOOLCH)/$(LINKER) -Itoolchain/Debian/Tools/Cross/i686-elf/lib/gcc/i686-elf/include -Isrc -g -finline-functions -ffreestanding -fbuiltin -Wall -Wextra
LLVMCFLAGS = -ffreestanding -nostdlib -nodefaultlibs -target i686-pc-none-elf -Wno-format -pedantic -fno-omit-frame-pointer -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -fno-exceptions -I/usr/lib/llvm-3.8/lib/clang/3.8.0/include -Isrc
ASFLAGS = 
NASFLAGS = -g -f elf32
CPPFLAGS_MODS = -Itoolchain/Debian/Tools/Cross/i686-elf/lib/gcc/i686-elf/include -Isrc -O2 -finline-functions -fstrength-reduce -ffreestanding -Wno-format -pedantic -fno-omit-frame-pointer -nostdlib -Wall -Wextra -lgcc -Wno-unused-function -Wno-unused-parameter -Wno-unknown-pragmas -std=c++11 -fno-exceptions

# Output constants (filenames and paths)
DISKPATH = iso
BOUT = obj
# Kernel filename:
KOUT = ksharp.bin
################# Includes #################

include src/subdir.mk
include src/task/subdir.mk
include src/kernel_headers/subdir.mk
include src/terminal/subdir.mk
include src/modules/subdir.mk
include src/modules/clock/subdir.mk
include src/modules/kbd/subdir.mk
include src/libc/subdir.mk
include src/libc/data_struct/subdir.mk
include src/memory/subdir.mk
include src/arch/x86/subdir.mk
include src/arch/x86/gdt/subdir.mk
include src/arch/x86/irq/subdir.mk
include src/arch/x86/isr/subdir.mk
include src/arch/x86/idt/subdir.mk
include src/version/subdir.mk

############### Main targets ###############

all: kernel-link

# Link all those subdir.mk object files into the whole Kernel:
kernel-link: $(OBJS) $(MODS)
	@echo '----------'
	@echo 'Toolchain: LLVM Toolchain'
	@echo '>>>> Linking Kernel <<<<'
	@echo '>>>> Invoking: LLVM C++ Linker <<<<'
	$(CXX_LLVM) -T $(TOOLCH)/$(LINKER) $(LLVMCPPFLAGS) -o $(DISKPATH)/$(KOUT) $(OBJS)
	@echo '>>>> Finished building target: $@ <<<<'
	@echo '----------'

clean:
	rm $(BOUT)/*.o
	rm $(BOUT)/modules/*.mod
