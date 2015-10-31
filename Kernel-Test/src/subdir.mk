OBJS += \
$(BOUT)\kmain.o

$(BOUT)\kmain.o: src\kmain.cpp
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<
	@echo '>> Finished building: $<'
	@echo ' '
