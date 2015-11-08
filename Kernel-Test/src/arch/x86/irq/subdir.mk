OBJS += \
$(BOUT)\irq_defs.o

$(BOUT)\irq_defs.o: src\arch\x86\irq\irq_defs.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
