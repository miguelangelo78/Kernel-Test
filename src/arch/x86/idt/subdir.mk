OBJS += \
$(BOUT)/idt.o

$(BOUT)/idt.o: src/arch/x86/idt/idt.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
