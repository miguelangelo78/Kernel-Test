OBJS +=
MODS += \
$(BOUT)\modules\pit.mod

$(BOUT)\modules\pit.mod: src\modules\pit.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o build/modules/pit.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
