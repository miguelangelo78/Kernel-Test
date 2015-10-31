cls
@echo off

printf "Cleaning built objects\n"
make -f toolchain\makefile.mak clean
if ERRORLEVEL 1 ( call:errorhandle "Object cleaning" )

python genmake.py
if ERRORLEVEL 1 ( call:errorhandle "Generating makefiles" )

call build.bat
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF