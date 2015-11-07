OBJS += \
$(BOUT)\idt_flush.o

$(BOUT)\idt_flush.o: src\arch\x86\idt\idt_flush.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
