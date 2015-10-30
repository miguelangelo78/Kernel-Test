cls
@echo off

python genmake.py
if ERRORLEVEL 1 ( call:errorhandle "Generating makefiles" )

call build.bat
exit

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF