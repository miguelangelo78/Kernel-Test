OBJS +=
MODS += \
$(BOUT)/modules/ext2.mod

$(BOUT)/modules/ext2.mod: src/modules/fs/ext2.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/ext2.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
