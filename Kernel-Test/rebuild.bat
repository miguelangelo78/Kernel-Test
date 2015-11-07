cls
@echo off

printf "Cleaning built objects\n"
make -f toolchain\makefile.mak clean

printf "Generating symbols from Kernel\n"
python gensym.py > src\symbols.s
if ERRORLEVEL 1 ( call:errorhandle "Generating symbols")

python genmake.py
if ERRORLEVEL 1 ( call:errorhandle "Generating makefiles" )

call build.bat
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF