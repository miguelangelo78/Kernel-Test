OBJS += \
$(BOUT)\mod_io.o

$(BOUT)\mod_io.o: src\kernel_headers\mod_io.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
