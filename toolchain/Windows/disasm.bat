@cd "%~dp0"
@echo off

objdump -M intel -d ../../iso/ksharp.bin > out.txt
out.txt
del out.txt
