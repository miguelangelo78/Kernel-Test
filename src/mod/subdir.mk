OBJS +=
MODS += \
$(BOUT)\modules\mod.mod

$(BOUT)\modules\mod.mod: src\mod\mod.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o build/modules/mod.mod -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
