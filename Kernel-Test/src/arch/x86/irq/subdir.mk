OBJS += \
$(BOUT)\irq.o \
$(BOUT)\irq_defs.o

$(BOUT)\irq.o: src\arch\x86\irq\irq.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\irq_defs.o: src\arch\x86\irq\irq_defs.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
