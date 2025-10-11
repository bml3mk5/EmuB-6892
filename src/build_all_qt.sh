#!/bin/sh

PROJNAME=bml3mk5_qt

make_all() {
	mkdir -p source/Qt/$2
	cd source/Qt/$2
	qmake ../$PROJNAME.pro $3 $4
	if [ "$1" = "" ]; then
		make clean
	fi
	make
	make install
	cd ../../..
}

make_linux_all() {
	mkdir -p source/Qt/$2
	cd source/Qt/$2
	qtchooser -run-tool=qmake -qt=6 ../$PROJNAME.pro $3 $4
	if [ "$1" = "" ]; then
		make clean
	fi
	make
	make install
	cd ../../..
}

svn update
UNAME=`uname -s`
UNAMEU=`echo $UNAME | cut -c 1-5`
if [ "$UNAME" = "Darwin" ]; then
	PATH=$PATH:~/Qt/5.12.4/clang_64/bin; export PATH
	PLATFORM=Desktop_Qt_5_12_4_clang_64bit
	SUBDIR=build-$PROJNAME-$PLATFORM-Release
	make_all "$1" $SUBDIR
	SUBDIR=build-$PROJNAME-$PLATFORM-Release-Dbgr
	make_all "$1" $SUBDIR CONFIG+=debugger
fi
if [ "$UNAME" = "Linux" ]; then
	if [ "$1" = "devel" ]; then
		PATH=$HOME/Qt/5.12.4/gcc_64/bin:$HOME/Qt/Tools/QtCreator/bin:$PATH; export PATH
		PLATFORM=Desktop_Qt_5_12_4_GCC_64bit
		SUBDIR=build-$PROJNAME-$PLATFORM-Release
		make_all "$2" $SUBDIR 
		SUBDIR=build-$PROJNAME-$PLATFORM-Release-Dbgr
		make_all "$2" $SUBDIR CONFIG+=debugger
	else
		PLATFORM=
		SUBDIR=Release
		make_linux_all "$1" $SUBDIR 
		SUBDIR=ReleaseDbgr
		make_linux_all "$1" $SUBDIR CONFIG+=debugger
	fi
fi
if [ "$UNAMEU" = "MINGW" ]; then
	PLATFORM=Desktop_Qt_5_12_4_MinGW_64_bit
	SUBDIR=build-$PLATFORM-release
	make_all "$1" $SUBDIR
fi

