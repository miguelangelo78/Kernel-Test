@echo off
setlocal EnableDelayedExpansion
@cd "%~dp0"
cls

set pkg_count=0 

rem !!!!!!!!!!!!!!!!!!!!!!!! CREATE PACKAGES HERE !!!!!!!!!!!!!!!!!!!!!!!!
call :NEW_PKG "Python 2.7.11" python-2.7.11.msi "python.exe -h" "https://www.python.org/ftp/python/2.7.11/python-2.7.11.msi"
call :NEW_PKG MinGW mingw-get-setup.exe "gcc.exe --help" "http://downloads.sourceforge.net/project/mingw/Installer/mingw-get-setup.exe?r=http%3A%2F%2Fwww.mingw.org%2Fdownload%2Finstaller%3F`&ts=1466972726`&use_mirror=ufpr"
call :NEW_PKG "Clang 3.8.0 (LLVM)" LLVM-3.8.0-win32.exe "clang.exe --help" "http://llvm.org/releases/3.8.0/LLVM-3.8.0-win32.exe"
call :NEW_PKG Git Git-2.9.0-64-bit.exe "git.exe --help" "https://github.com/git-for-windows/git/releases/download/v2.9.0.windows.1/Git-2.9.0-32-bit.exe"
call :NEW_PKG QEMU qemu-w32-setup-20160523.exe "qemu-img.exe --help" "http://qemu.weilnetz.de/w32/qemu-w32-setup-20160523.exe"
rem !!!!!!!!!!!!!!!!!!!!!!!! CREATE PACKAGES HERE !!!!!!!!!!!!!!!!!!!!!!!!

echo ********* Kernel Development Toolkit Downloader *********
echo * The script will now download and install the following packages:
for /L %%i in (1,1,%pkg_count%) do (
	call echo %%i- %%pkg[%%i].Name%%
)

:PROMPT
SET /P ACCEPT="* Do you accept these changes [Y]/n: "
IF /I "%ACCEPT%" NEQ "Y" GOTO END

echo. && echo ***** Step 1: Downloading Packages *****

for /L %%i in (1,1,%pkg_count%) do (
	call echo     %%i- Downloading %%pkg[%%i].Name%% . . .
	!pkg[%%i].Exec! >nul 2>&1 && (
    		echo     The program is already installed
	) || (
    		if NOT exist "!pkg[%%i].Setup!" (
			powershell -Command "(New-Object Net.WebClient).DownloadFile('!pkg[%%i].URL!', '!pkg[%%i].Setup!')"
			echo     Done
		) else (
			echo     File already exists
		)
	)
	echo.
)

echo ***** Step 2: Installing Packages *****
for /L %%i in (1,1,%pkg_count%) do (
	if exist "!pkg[%%i].Setup!" (
		call echo     %%i- Installing %%pkg[%%i].Name%% . . .
		!pkg[%%i].Setup!
	)
)

echo. && echo ***** Step 3: Setting Environment Variables *****
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_SZ /d "%path%;C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Python27;C:\Program Files (x86)\qemu"

echo. && echo ***** Step 4: Installing Kernel Source Code *****
git clone https://github.com/miguelangelo78/Kernel-Test.git
cd Kernel-Test
git pull
cd ..

echo ***** Step 5: Success! ***** && echo.

:PROMPT
SET /P CLEAN_ACCEPT="* Do you wish to clean up the downloaded files [Y]/n: "
IF /I "%CLEAN_ACCEPT%" NEQ "Y" GOTO INSTALLEND

echo * Deleting downloaded files
for /L %%i in (1,1,%pkg_count%) do (
	call echo     %%i- Deleting %%pkg[%%i].Name%% . . .
	del "!pkg[%%i].Setup!"
)

:INSTALLEND
echo. && echo ** Download and Installation complete! **
PAUSE
GOTO END

:NEW_PKG

set /A pkg_count=%pkg_count%+1
set pkg[%pkg_count%].Name=%~1
set pkg[%pkg_count%].Setup=%~2
set pkg[%pkg_count%].Exec=%~3
set pkg[%pkg_count%].URL="%~4"
goto:EOF

:END
endlocal