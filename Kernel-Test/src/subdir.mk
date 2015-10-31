OBJS += \
$(BOUT)\foo.o \
$(BOUT)\kmain.o

$(BOUT)\foo.o: src\foo.cpp
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<
	@echo '>> Finished building: $<'
	@echo ' '

$(BOUT)\kmain.o: src\kmain.cpp
	@echo '>> Building file $<'
	@echo '>> Invoking Cross i686-elf GCC Compiler'
	$(CXX) $(CFLAGS)  -o $@ -c $<
	@echo '>> Finished building: $<'
	@echo ' '
