cls
@echo off

printf "Generating symbols from Kernel\n"
python gensym.py > src\symbols.s
if ERRORLEVEL 1 ( call:errorhandle "Generating symbols")

python genmake.py
if ERRORLEVEL 1 ( call:errorhandle "Generating makefiles" )

@REM calling rebuild.bat for the 1st time
IF [%1]==[] ( call build.bat 0 )

@REM calling rebuild.bat FROM build.bat
call build.bat %1
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF