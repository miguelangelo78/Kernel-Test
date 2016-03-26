OBJS +=
MODS += \
$(BOUT)\modules\mod.mod \
$(BOUT)\modules\mod2.mod

$(BOUT)\modules\mod.mod: src\mod\mod.cpp build\modules\mod2.mod
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX) $(CPPFLAGS_MODS) -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o build/modules/mod.mod  $< build\modules\mod2.mod 
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\modules\mod2.mod: src\mod\mod2.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS) -fno-zero-initialized-in-bss -O2 -W -Wall -Wstrict-prototypes -Wmissing-prototypes -D__KERNEL__ -DMODULE -o build/modules/mod2.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
