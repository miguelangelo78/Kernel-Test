OBJS += \
$(BOUT)\isr.o \
$(BOUT)\isr_defs.o

$(BOUT)\isr.o: src\arch\x86\isr\isr.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\isr_defs.o: src\arch\x86\isr\isr_defs.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
