OBJS += \
$(BOUT)\ksharp_stage2.o

$(BOUT)\ksharp_stage2.o: src\arch\x86\ksharp_stage2.asm 
	@echo '>> Building file $<'
	@echo '>> Invoking NASM Assembler'
	$(NAS) $(NASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
