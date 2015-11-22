@cd "%~dp0"
cls
@echo off

python genmake.py
if ERRORLEVEL 1 ( call:errorhandle "Generating makefiles" )

@REM calling rebuild.bat for the 1st time
IF [%1]==[] ( call build.bat 0 )

@REM calling rebuild.bat FROM build.bat
call build.bat %1
@GOTO:EOF

:errorhandle
printf "****\nERROR: %~1\n****\n"
pause
exit
GOTO:EOF