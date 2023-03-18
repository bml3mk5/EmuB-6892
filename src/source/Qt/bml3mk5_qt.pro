#-------------------------------------------------
#
# Project created by QtCreator 2016-11-17T18:52:02
#
#-------------------------------------------------

QT       += core gui multimedia opengl serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
lessThan(QT_MAJOR_VERSION, 6): QT += gamepad

TARGET = bml3mk5_qt
TEMPLATE = app

TRANSLATIONS = ../locale/ja/bml3mk5_ja.ts
PO_FILE = ../locale/ja/LC_MESSAGES/bml3mk5.po

DEFINES += _BML3MK5 USE_OPENGL GUI_TYPE_QT USE_QT
# application treats _TCHAR as narrow char
win32:DEFINES -= UNICODE _UNICODE

CONFIG(debug, debug|release) {
	DEFINES += _DEBUG _DEBUG_LOG
}

debugger {
	DEFINES += USE_DEBUGGER
}

INCLUDEPATH += include
win32:INCLUDEPATH += /Devel/include/ffmpeg341
win32:INCLUDEPATH += /Devel/include
!win32:INCLUDEPATH += /usr/local/include
!win32:INCLUDEPATH += $(HOME)/Devel/ffmpeg

# RC_ICONS = ../src/res/windows/bml3mk5.ico
RC_FILE = ../src/res/windows/bml3mk5.rc

SOURCES +=\
	../src/ConvertUTF.c \
	../src/cchar.cpp \
	../src/cmutex.cpp \
	../src/common.cpp \
	../src/config.cpp \
	../src/curtime.cpp \
	../src/cpixfmt.cpp \
	../src/debugger_bpoint.cpp \
	../src/debugger_socket.cpp \
	../src/debugger_symbol.cpp \
	../src/depend.cpp \
	../src/emumsg.cpp \
	../src/fifo.cpp \
	../src/fileio.cpp \
	../src/simple_ini.cpp \
	../src/utility.cpp \
	../src/labels.cpp \
	../src/msgs.cpp \
	../src/osd/qt/qt_clocale.cpp \
	../src/osd/qt/qt_cbitmap.cpp \
	../src/osd/qt/qt_ccolor.cpp \
	../src/osd/qt/qt_csurface.cpp \
	../src/osd/qt/qt_msgboard.cpp \
	../src/osd/qt/qt_parseopt.cpp \
	../src/osd/qt/qt_input.cpp \
	../src/osd/qt/qt_input_keysym.cpp \
	../src/osd/qt/qt_main.cpp \
	../src/osd/qt/qt_screen.cpp \
	../src/osd/qt/qt_screenmode.cpp \
	../src/osd/qt/qt_sound.cpp \
	../src/osd/qt/qt_timer.cpp \
	../src/osd/qt/qt_socket.cpp \
	../src/osd/qt/qt_utils.cpp \
	../src/osd/qt/qt_ledboxbase.cpp \
	../src/osd/qt/qt_vkeyboardbase.cpp \
	../src/osd/qt/qt_uart.cpp \
	../src/osd/qt/qt_emu.cpp \
	../src/osd/qt/qt_debugger_console.cpp \
	../src/osd/d88_files.cpp \
	../src/osd/debugger_console.cpp \
	../src/osd/disk_parser.cpp \
	../src/osd/emu.cpp \
	../src/osd/emu_input.cpp \
	../src/osd/emu_input_keysym.cpp \
	../src/osd/emu_screen.cpp \
	../src/osd/emu_sound.cpp \
	../src/osd/opengl.cpp \
	../src/osd/logging.cpp \
	../src/osd/screenmode.cpp \
	../src/osd/windowmode.cpp \
	../src/gui/gui_base.cpp \
	../src/gui/gui_keybinddata.cpp \
	../src/gui/qt/qt_dialog.cpp \
	../src/gui/qt/qt_aboutbox.cpp \
	../src/gui/qt/qt_configbox.cpp \
	../src/gui/qt/qt_gui.cpp \
	../src/gui/qt/qt_keybindbox.cpp \
	../src/gui/qt/qt_ledbox.cpp \
	../src/gui/qt/qt_vkeyboard.cpp \
	../src/gui/qt/qt_volumebox.cpp \
	../src/gui/qt/qt_recvidbox.cpp \
	../src/gui/qt/qt_recaudbox.cpp \
	../src/video/ffmpeg/ffm_loadlib.cpp \
	../src/video/ffmpeg/ffm_rec_audio.cpp \
	../src/video/ffmpeg/ffm_rec_video.cpp \
	../src/video/qt/qt_bitmap.cpp \
	../src/video/wave/wav_rec_audio.cpp \
	../src/video/rec_audio.cpp \
	../src/video/rec_video.cpp \
	../src/vm/acia.cpp \
	../src/vm/ay38910.cpp \
	../src/vm/debugger.cpp \
	../src/vm/debugger_base.cpp \
	../src/vm/device.cpp \
	../src/vm/disk.cpp \
	../src/vm/event.cpp \
	../src/vm/hd46505.cpp \
	../src/vm/mb8866.cpp \
	../src/vm/mc6809.cpp \
	../src/vm/mc6809dasm.cpp \
	../src/vm/mc6843.cpp \
	../src/vm/msm58321.cpp \
	../src/vm/noise.cpp \
	../src/vm/parsewav.cpp \
	../src/vm/paw_datas.cpp \
	../src/vm/paw_defs.cpp \
	../src/vm/paw_dft.cpp \
	../src/vm/paw_file.cpp \
	../src/vm/paw_format.cpp \
	../src/vm/paw_param.cpp \
	../src/vm/paw_parse.cpp \
	../src/vm/paw_parsecar.cpp \
	../src/vm/paw_parsewav.cpp \
	../src/vm/paw_util.cpp \
	../src/vm/pia.cpp \
	../src/vm/via.cpp \
	../src/vm/bml3mk5/bml3mk5.cpp \
	../src/vm/bml3mk5/board.cpp \
	../src/vm/bml3mk5/cmt.cpp \
	../src/vm/bml3mk5/comm.cpp \
	../src/vm/bml3mk5/display.cpp \
	../src/vm/bml3mk5/floppy.cpp \
	../src/vm/bml3mk5/kanji.cpp \
	../src/vm/bml3mk5/keyboard.cpp \
	../src/vm/bml3mk5/keyrecord.cpp \
	../src/vm/bml3mk5/l3basic.cpp \
	../src/vm/bml3mk5/memory.cpp \
	../src/vm/bml3mk5/printer.cpp \
	../src/vm/bml3mk5/psg9c.cpp \
	../src/vm/bml3mk5/psgc.cpp \
	../src/vm/bml3mk5/registers.cpp \
	../src/vm/bml3mk5/rtc.cpp \
	../src/vm/bml3mk5/sound.cpp \
	../src/vm/bml3mk5/timer.cpp \
	../src/vm/fmgen/fmgen.cpp \
	../src/vm/fmgen/fmtimer.cpp \
	../src/vm/fmgen/opm.cpp \
	../src/vm/fmgen/opna.cpp \
	../src/vm/fmgen/psg.cpp

win32:SOURCES += ../src/video/vfw/vfw_rec_video.cpp

macx:SOURCES += ../src/video/avkit/avk_rec_common.mm \
	../src/video/avkit/avk_rec_video.mm \
	../src/video/avkit/avk_rec_audio.mm

HEADERS  +=\
	../src/ConvertUTF.h \
	../src/cbitmap.h \
	../src/cchar.h \
	../src/ccolor.h \
	../src/clocale.h \
	../src/cmutex.h \
	../src/common.h \
	../src/config.h \
	../src/cptrlist.h \
	../src/csurface.h \
	../src/cpixfmt.h \
	../src/d88_defs.h \
	../src/debugger_bpoint.h \
	../src/debugger_defs.h \
	../src/debugger_socket.h \
	../src/debugger_symbol.h \
	../src/depend.h \
	../src/emu.h \
	../src/emu_osd.h \
	../src/emumsg.h \
	../src/fifo.h \
	../src/fileio.h \
	../src/keycode.h \
	../src/labels.h \
	../src/loadlibrary.h \
	../src/logging.h \
	../src/msgboard.h \
	../src/parseopt.h \
	../src/osd/d88_files.h \
	../src/osd/debugger_console.h \
	../src/osd/disk_parser.h \
	../src/osd/emu_input.h \
	../src/osd/opengl.h \
	../src/osd/qt/qt_cbitmap.h \
	../src/osd/qt/qt_clocale.h \
	../src/osd/qt/qt_ccolor.h \
	../src/osd/qt/qt_csurface.h \
	../src/osd/qt/qt_emu.h \
	../src/osd/qt/qt_msgboard.h \
	../src/osd/qt/qt_parseopt.h \
	../src/osd/qt/qt_input.h \
	../src/osd/qt/qt_screenmode.h \
	../src/osd/qt/qt_sound.h \
	../src/osd/qt/qt_socket.h \
	../src/osd/qt/qt_utils.h \
	../src/osd/qt/qt_ledboxbase.h \
	../src/osd/qt/qt_vkeyboardbase.h \
	../src/osd/qt/qt_uart.h \
	../src/osd/qt/qt_main.h \
	../src/osd/qt/qt_common.h \
	../src/osd/qt/qt_debugger_console.h \
	../src/osd/screenmode.h \
	../src/osd/windowmode.h \
	../src/rec_video_defs.h \
	../src/utils.h \
	../src/res/resource.h \
	../src/simple_ini.h \
	../src/utility.h \
	../src/msgs.h \
	../src/version.h \
	../src/gui/qt/qt_dialog.h \
	../src/gui/qt/qt_aboutbox.h \
	../src/gui/qt/qt_configbox.h \
	../src/gui/qt/qt_gui.h \
	../src/gui/qt/qt_keybindbox.h \
	../src/gui/qt/qt_ledbox.h \
	../src/gui/qt/qt_vkeyboard.h \
	../src/gui/qt/qt_volumebox.h \
	../src/gui/qt/qt_recvidbox.h \
	../src/gui/qt/qt_recaudbox.h \
	../src/gui/gui.h \
	../src/gui/gui_base.h \
	../src/gui/gui_keybinddata.h \
	../src/gui/ledbox.h \
	../src/gui/vkeyboard.h \
	../src/gui/vkeyboard_bml3mk5.h \
	../src/video/ffmpeg/ffm_loadlib.h \
	../src/video/ffmpeg/ffm_rec_audio.h \
	../src/video/ffmpeg/ffm_rec_video.h \
	../src/video/wave/wav_rec_audio.h \
	../src/video/qt/qt_bitmap.h \
	../src/video/rec_audio.h \
	../src/video/rec_video.h \
	../src/vm/acia.h \
	../src/vm/ay38910.h \
	../src/vm/debugger.h \
	../src/vm/debugger_base.h \
	../src/vm/device.h \
	../src/vm/disk.h \
	../src/vm/event.h \
	../src/vm/floppy_defs.h \
	../src/vm/hd46505.h \
	../src/vm/mb8866.h \
	../src/vm/mc6809.h \
	../src/vm/mc6809_consts.h \
	../src/vm/mc6809dasm.h \
	../src/vm/mc6843.h \
	../src/vm/noise.h \
	../src/vm/parsewav.h \
	../src/vm/paw_datas.h \
	../src/vm/paw_defs.h \
	../src/vm/paw_dft.h \
	../src/vm/paw_file.h \
	../src/vm/paw_param.h \
	../src/vm/paw_parse.h \
	../src/vm/paw_parsecar.h \
	../src/vm/paw_parsewav.h \
	../src/vm/paw_util.h \
	../src/vm/pia.h \
	../src/vm/via.h \
	../src/vm/vm.h \
	../src/vm/bml3mk5/bml3mk5.h \
	../src/vm/bml3mk5/bml3mk5_defs.h \
	../src/vm/bml3mk5/board.h \
	../src/vm/bml3mk5/cmt.h \
	../src/vm/bml3mk5/comm.h \
	../src/vm/bml3mk5/display.h \
	../src/vm/bml3mk5/displaysub.h \
	../src/vm/bml3mk5/floppy.h \
	../src/vm/bml3mk5/kanji.h \
	../src/vm/bml3mk5/keyboard.h \
	../src/vm/bml3mk5/keyboard_bind.h \
	../src/vm/bml3mk5/keyrecord.h \
	../src/vm/bml3mk5/memory.h \
	../src/vm/bml3mk5/memory_readio.h \
	../src/vm/bml3mk5/memory_writeio.h \
	../src/vm/bml3mk5/printer.h \
	../src/vm/bml3mk5/psg9c.h \
	../src/vm/bml3mk5/psgc.h \
	../src/vm/bml3mk5/registers.h \
	../src/vm/bml3mk5/sound.h \
	../src/vm/bml3mk5/timer.h \
	../src/vm/fmgen/diag.h \
	../src/vm/fmgen/fmgen.h \
	../src/vm/fmgen/fmgeninl.h \
	../src/vm/fmgen/fmtimer.h \
	../src/vm/fmgen/headers.h \
	../src/vm/fmgen/misc.h \
	../src/vm/fmgen/opm.h \
	../src/vm/fmgen/opna.h \
	../src/vm/fmgen/psg.h \
	../src/vm/fmgen/types.h

win32:HEADERS += ../src/video/vfw/vfw_rec_video.h

!win32:HEADERS += ../src/extra/tchar.h

macx:HEADERS += ../src/video/avkit/avk_rec_common.h \
	../src/video/avkit/avk_rec_video.h \
	../src/video/avkit/avk_rec_audio.h

FORMS    += \
	../src/gui/qt/qt_aboutbox.ui \
	../src/gui/qt/qt_gui.ui \
	../src/gui/qt/qt_keybindbox.ui

win32-msvc:LIBS += opengl32.lib user32.lib
win32-g++:LIBS += -lopengl32 -luser32 -lvfw32
macx:LIBS += -framework Cocoa -framework AVFoundation -framework CoreMedia -framework CoreVideo -framework CoreAudio
linux:LIBS += -ldl

DISTFILES += \
	../src/res/windows/bml3mk5.ico \
	../src/res/windows/bml3mk5.rc

copy_res_win32.path = $$OUT_PWD/release/res
copy_qm_win32.path = $$OUT_PWD/release/locale
CONFIG(debug, debug|release) {
copy_res_win32.path = $$OUT_PWD/debug/res
copy_qm_win32.path = $$OUT_PWD/debug/locale
}
copy_res_win32.files = ../src/res/common/*.*
copy_qm_win32.files = ../locale/list.xml ../locale/ja/*.qm

win32:INSTALLS += copy_res_win32 copy_qm_win32

copy_res_macx.path = $$OUT_PWD/$${TARGET}.app/Contents/Resources
copy_qm_macx.path = $$OUT_PWD/$${TARGET}.app/Contents/Resources/locale
copy_res_macx.files = ../src/res/common/*.*
copy_qm_macx.files = ../locale/list.xml ../locale/ja/*.qm

macx:INSTALLS += copy_res_macx copy_qm_macx

copy_res_linux.path = $$OUT_PWD/res
copy_qm_linux.path = $$OUT_PWD/locale
copy_res_linux.files = ../src/res/common/*.*
copy_qm_linux.files = ../locale/list.xml ../locale/ja/*.qm

linux:INSTALLS += copy_res_linux copy_qm_linux
