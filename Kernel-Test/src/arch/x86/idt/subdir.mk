OBJS += \
$(BOUT)\idt.o

$(BOUT)\idt.o: src\arch\x86\idt\idt.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
