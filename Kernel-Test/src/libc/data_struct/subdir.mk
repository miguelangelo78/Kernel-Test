OBJS += \
$(BOUT)\hashmap.o \
$(BOUT)\list.o \
$(BOUT)\tree.o

$(BOUT)\hashmap.o: src\libc\data_struct\hashmap.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\list.o: src\libc\data_struct\list.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\tree.o: src\libc\data_struct\tree.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking LLVM C++ Clang++'
	$(CXX_LLVM) $(LLVMCPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
