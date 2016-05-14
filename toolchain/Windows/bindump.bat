@cd "%~dp0"
@echo off

readelf -a ../../iso/ksharp.bin > out.txt
out.txt
del out.txt