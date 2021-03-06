OBJS +=
MODS += \
$(BOUT)/modules/ext2.mod \
$(BOUT)/modules/pipe.mod \
$(BOUT)/modules/unixpipe.mod

$(BOUT)/modules/ext2.mod: src/modules/fs/ext2.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/ext2.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/modules/pipe.mod: src/modules/fs/pipe.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/pipe.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/modules/unixpipe.mod: src/modules/fs/unixpipe.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o obj/modules/unixpipe.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
