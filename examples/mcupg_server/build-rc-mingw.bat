@echo off

set target=%1
if "%target%" == "" (
	set target=mcupg_server
)
set resources=mcupg_server.rc
set incs=-I..\..\libiup\include -L..\..\libiup\mingw64
set cflags=-Wl,-subsystem,windows -liup -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32 -liphlpapi -lws2_32

rem rmdir /s /q %target%.exe
rem del /q %target%.exe
rem call :my_sleep

echo "windres %resources% -o %resources%.o"
windres %resources% -o %resources%.o
call :my_sleep

echo "gcc -c %target%.c -o %target%.o %incs% %cflags%"
gcc -c %target%.c -o %target%.o %incs% %cflags%
call :my_sleep

echo "gcc %target%.o %resources%.o -o %target%.exe %incs% %cflags%"
gcc %target%.o %resources%.o -o %target%.exe %incs% %cflags%
call :my_sleep

strip %target%.exe
call :my_sleep

upx %target%.exe
goto :eof

:my_sleep
rem echo "wait 1s ..."
ping 127.0.0.1 -n 2 > nul
goto :eof
