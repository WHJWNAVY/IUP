@echo off

set target=%1
if "%target%" == "" (
	set target=sample
)
set incs=-I..\..\libiup\include -L..\..\libiup\mingw64
set cflags=-Wl,-subsystem,windows -liup -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32

rem rmdir /s /q %target%.exe
rem del /q %target%.exe
rem call :my_sleep

echo "gcc -o %target%.exe %target%.c %incs% %cflags%"
gcc -o %target%.exe %target%.c %incs% %cflags%
call :my_sleep

strip %target%.exe
call :my_sleep

upx %target%.exe
goto :eof

:my_sleep
rem echo "wait 1s ..."
ping 127.0.0.1 -n 2 > nul
goto :eof
