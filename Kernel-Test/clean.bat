@cd "%~dp0"
cls
@echo off

printf "Cleaning built objects\n"
make -f toolchain\makefile.mak clean

@GOTO:EOF