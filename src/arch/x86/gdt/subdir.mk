OBJS += \
$(BOUT)/gdt.o \
$(BOUT)/gdt_flush.o

$(BOUT)/gdt.o: src/arch/x86/gdt/gdt.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/gdt_flush.o: src/arch/x86/gdt/gdt_flush.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
