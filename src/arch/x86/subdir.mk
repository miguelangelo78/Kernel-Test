OBJS += \
$(BOUT)\startup_stg2.o

$(BOUT)\startup_stg2.o: src\arch\x86\startup_stg2.asm 
	@echo '>> Building file $<'
	@echo '>> Invoking NASM Assembler'
	$(NAS) $(NASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
