OBJS += \
$(BOUT)/syscall_vector.o \
$(BOUT)/syscall.o

$(BOUT)/syscall_vector.o: src/syscall/syscall_vector.c 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C Clang'
	$(CC_LLVM) $(LLVMCFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/syscall.o: src/syscall/syscall.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
