#!/bin/sh
# $1 = project
# $2 = __WXMSW__

pwd

windres.exe ../../../src/res/windows/$1.rc -D$2 "-I$3" "-I$4" -O coff -o $1.res

