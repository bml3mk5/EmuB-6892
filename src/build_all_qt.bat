rem @echo off

set path=%path%;D:\Qt\5.15.2\msvc2019_64\bin
set path=%path%;D:\Qt\Tools\QtCreator\bin
set path=%path%;D:\Qt\Tools\QtCreator\bin\jom

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

svn update

set appname=bml3mk5_qt
set subdir=build-Desktop_Qt_5_15_2_MSVC2019_64bit

call :build_qt Release
call :build_qt Release _Dbgr "CONFIG+=debugger"

goto eof

:build_qt

md source\%subdir%-%1%2
cd source\%subdir%-%1%2

qmake.exe ..\Qt\%appname%.pro %3 %4

jom clean
jom
jom install

cd ..\..

:eof
