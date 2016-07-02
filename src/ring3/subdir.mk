OBJS += \
$(BOUT)/usermode.o \
$(BOUT)/usermode_enter.o

$(BOUT)/usermode.o: src/ring3/usermode.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/usermode_enter.o: src/ring3/usermode_enter.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
