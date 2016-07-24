OBJS += \
$(BOUT)/libcc.o \
$(BOUT)/kprintf.o \
$(BOUT)/libc.o \
$(BOUT)/string.o
LIBRARIES += \
$(BOUT)/slre.o

$(BOUT)/libcc.o: src/libc/libcc.c 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C Clang'
	$(CC_LLVM) $(LLVMCFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/kprintf.o: src/libc/kprintf.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/libc.o: src/libc/libc.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/slre.o: src/libc/slre.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)/string.o: src/libc/string.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
