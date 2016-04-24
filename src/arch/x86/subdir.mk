OBJS += \
$(BOUT)/vm8086.o \
$(BOUT)/startup_stg2.o

$(BOUT)/vm8086.o: src/arch/x86/vm8086.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/startup_stg2.o: src/arch/x86/startup_stg2.asm 
	@echo '>> Building file $<'
	@echo '>> Invoking NASM Assembler'
	$(NAS) $(NASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
