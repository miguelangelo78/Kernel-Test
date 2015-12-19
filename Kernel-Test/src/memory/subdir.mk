OBJS += \
$(BOUT)\kmemory.o \
$(BOUT)\mem_copy_page_phys.o

$(BOUT)\kmemory.o: src\memory\kmemory.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CPPFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\mem_copy_page_phys.o: src\memory\mem_copy_page_phys.s 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Assembler'
	$(AS) $(ASFLAGS)  -o $@  $<  
	@echo '>> Finished building: $<'
	@echo ' '
