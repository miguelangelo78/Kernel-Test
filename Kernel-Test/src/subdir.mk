OBJS += \
$(BOUT)\kmain.o \
$(BOUT)\foo.o

$(BOUT)\kmain.o: $(SRCPATH)\kmain.cpp
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS) -o $@ -c $<
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\foo.o: $(SRCPATH)\foo.cpp
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS) -o $@ -c $<
	@echo '>> Finished building: $<'
	@echo ' '