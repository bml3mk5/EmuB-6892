rem @echo off

set path=%path%;D:\Qt\5.12.4\mingw73_64\bin;D:\Qt\Tools\mingw730_64\bin;D:\Qt\Tools\mingw730_64\x86_64-w64-mingw32\bin
set path=%path%;D:\Qt\Tools\QtCreator\bin

svn update

set appname=bml3mk5_qt
set subdir=build-Desktop_Qt_5_12_4_MinGW_64_bit

call :build_qt release
call :build_qt release -dbgr "CONFIG+=debugger"

goto eof

:build_qt

md source\Qt\%subdir%-%1%2
cd source\Qt\%subdir%-%1%2

qmake.exe ..\%appname%.pro %3 %4

jom %1-clean
jom %1
jom %1-install

cd ..\..\..

:eof
