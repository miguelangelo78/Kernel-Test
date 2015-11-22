@cd "%~dp0"
cls
@echo off

python genmake.py
if ERRORLEVEL 1 ( call:errorhandle "Generating makefiles" )

call build.bat %1
@GOTO:EOF

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF