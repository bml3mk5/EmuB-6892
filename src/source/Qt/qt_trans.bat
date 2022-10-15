rem @echo off

set path=%path%;D:\Qt\5.12.4\msvc2017_64\bin
set path=%path%;D:\Qt\Tools\QtCreator\bin

lupdate -tr-function-alias QT_TR_NOOP+=_TX bml3mk5_qt.pro
lrelease bml3mk5_qt.pro
