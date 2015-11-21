OBJS += \
$(BOUT)\kmemory.o

$(BOUT)\kmemory.o: src\memory\kmemory.cpp 
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<  
	@echo '>> Finished building: $<'
	@echo ' '
