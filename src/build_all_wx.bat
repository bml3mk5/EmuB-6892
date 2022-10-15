rem @echo off

set app=bml3mk5
set path=%path%;"C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE";C:\Windows\Microsoft.NET\Framework\v4.0.30319

if not "%1"=="" set tag=/t:%1

svn update

cd source\VC++2010

rem msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx;Platform=Win32
rem msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx;Platform=x64

rem msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx_Dbgr;Platform=Win32
rem msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx_Dbgr;Platform=x64

msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx2;Platform=Win32
msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx2;Platform=x64

msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx2_Dbgr;Platform=Win32
msbuild.exe %app%_wx.vcxproj %tag% /p:Configuration=Release_Wx2_Dbgr;Platform=x64

cd ..\..
