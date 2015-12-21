OBJS += \
$(BOUT)\args.o \
$(BOUT)\error.o \
$(BOUT)\kmain.o \
$(BOUT)\spin.o \
$(BOUT)\terminal.o \
$(BOUT)\vfs.o

$(BOUT)\args.o: src\args.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\error.o: src\error.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\kmain.o: src\kmain.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\spin.o: src\spin.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\terminal.o: src\terminal.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\vfs.o: src\vfs.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
