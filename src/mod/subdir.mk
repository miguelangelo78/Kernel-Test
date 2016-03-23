OBJS +=
MODS += \
$(BOUT)\modules\mod.mod

$(BOUT)\modules\mod.mod: src\mod\mod.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o build/modules/mod.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
