OBJS +=
MODS += \
$(BOUT)/modules/keyboard.mod

$(BOUT)/modules/keyboard.mod: src/modules/kbd/keyboard.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o build/modules/keyboard.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
