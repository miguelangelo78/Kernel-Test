OBJS += \
$(BOUT)\gdt_flush.o

$(BOUT)\gdt_flush.o: src\arch\x86\gdt\gdt_flush.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
