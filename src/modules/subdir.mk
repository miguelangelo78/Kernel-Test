OBJS +=
MODS += \
$(BOUT)\modules\fpu.mod

$(BOUT)\modules\fpu.mod: src\modules\fpu.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -r -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o build/modules/fpu.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
