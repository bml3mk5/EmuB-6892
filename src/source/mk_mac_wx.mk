#
# for MacOSX  
#
# export GUI_TYPE
# export BUILDDIR
# export LIBS
# export LDFLAGS
# export CDEFS
# export CFLAGS
# export CXXFLAGS
# export INSTALLDIR
# export ARCH

CC:=gcc
CXX:=g++
LD:=g++

FFMPEGDIR:=$(HOME)/Devel/ffmpeg

MACMINVER:=-mmacosx-version-min=10.7

COMMONCFLAGS:=-I/usr/local/include -I/usr/X11/include -I$(FFMPEGDIR) -I./include

COMMONLDFLAGS:=-lm -lz -lbz2 -liconv -Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreAudio -Wl,-framework,IOKit -Wl,-framework,AudioUnit -Wl,-framework,ForceFeedback -Wl,-framework,QTKit -Wl,-framework,AVFoundation -Wl,-framework,CoreMedia -Wl,-framework,CoreVideo -Wl,-framework,CoreAudio

CFLAGS:=$(CFLAGS) $(COMMONCFLAGS) $(MACMINVER)
CXXFLAGS:=$(CXXFLAGS) $(COMMONCFLAGS) $(MACMINVER)
LDFLAGS:=$(LDFLAGS) $(COMMONLDFLAGS) $(MACMINVER)
# OBJC_CFLAGS:=-fobjc-call-cxx-cdtors

EXEFILE:=bml3mk5

DATADIR:=data

SRCDIR:=src
SRCOSD:=$(SRCDIR)/osd
SRCOSDSDL:=$(SRCOSD)/SDL
SRCOSDWX:=$(SRCOSD)/wxwidgets
SRCOSDMAC:=$(SRCOSD)/mac
SRCVM:=$(SRCDIR)/vm
SRCFMGEN:=$(SRCVM)/fmgen
SRCDEP:=$(SRCVM)/bml3mk5
SRCVID:=$(SRCDIR)/video
SRCVIDWAV:=$(SRCVID)/wave
SRCVIDQTKIT:=$(SRCVID)/qtkit
SRCVIDAVKIT:=$(SRCVID)/avkit
SRCVIDFFM:=$(SRCVID)/ffmpeg
SRCVIDWX:=$(SRCVID)/wxwidgets
SRCGUIWX:=$(SRCDIR)/gui/wxwidgets

SRCRES:=$(SRCDIR)/res/common
MACRESDIR:=$(SRCDIR)/res/macosx

LOCALEDIR:=locale
SRCLOCALE:=$(LOCALEDIR)

EXE:=$(BUILDDIR)/$(EXEFILE)

EMUOBJS:=$(SRCDIR)/config.o \
	$(SRCDIR)/fifo.o \
	$(SRCDIR)/fileio.o \
	$(SRCDIR)/emumsg.o \
	$(SRCDIR)/common.o \
	$(SRCDIR)/depend.o \
	$(SRCDIR)/cchar.o \
	$(SRCDIR)/cmutex.o \
	$(SRCDIR)/curtime.o \
	$(SRCDIR)/cpixfmt.o \
	$(SRCDIR)/labels.o \
	$(SRCDIR)/msgs.o \
	$(SRCDIR)/simple_ini.o \
	$(SRCDIR)/ConvertUTF.o \
	$(SRCDIR)/debugger_bpoint.o \
	$(SRCDIR)/debugger_socket.o \
	$(SRCDIR)/debugger_symbol.o \
	$(SRCDIR)/utility.o

EMUOSDOBJS:=$(SRCOSDWX)/wxw_emu.o \
	$(SRCOSDWX)/wxw_csurface.o \
	$(SRCOSDWX)/wxw_cbitmap.o \
	$(SRCOSDWX)/wxw_ccolor.o \
	$(SRCOSDWX)/wxw_debugger_console.o \
	$(SRCOSDWX)/wxw_msgboard.o \
	$(SRCOSDWX)/wxw_ledboxbase.o \
	$(SRCOSDWX)/wxw_vkeyboardbase.o \
	$(SRCOSDWX)/wxw_clocale.o \
	$(SRCOSDWX)/wxw_input.o \
	$(SRCOSDWX)/wxw_input_keysym.o \
	$(SRCOSDWX)/wxw_screen.o \
	$(SRCOSDWX)/wxw_screenmode.o \
	$(SRCOSDWX)/wxw_socket.o \
	$(SRCOSDWX)/wxw_timer.o \
	$(SRCOSDWX)/wxw_main.o \
	$(SRCOSDSDL)/sdl_sound.o \
	$(SRCOSDMAC)/mac_uart.o \
	$(SRCOSD)/d88_files.o \
	$(SRCOSD)/debugger_console.o \
	$(SRCOSD)/disk_parser.o \
	$(SRCOSD)/emu.o \
	$(SRCOSD)/emu_input.o \
	$(SRCOSD)/emu_input_keysym.o \
	$(SRCOSD)/emu_screen.o \
	$(SRCOSD)/emu_sound.o \
	$(SRCOSD)/logging.o \
	$(SRCOSD)/screenmode.o \
	$(SRCOSD)/windowmode.o \
	$(SRCOSD)/opengl.o

VMOBJS:=$(SRCVM)/device.o \
	$(SRCVM)/event.o \
	$(SRCVM)/hd46505.o \
	$(SRCVM)/acia.o \
	$(SRCVM)/ay38910.o \
	$(SRCVM)/disk.o \
	$(SRCVM)/mb8866.o \
	$(SRCVM)/mc6809.o \
	$(SRCVM)/mc6809dasm.o \
	$(SRCVM)/mc6843.o \
	$(SRCVM)/msm58321.o \
	$(SRCVM)/noise.o \
	$(SRCVM)/pia.o \
	$(SRCVM)/via.o \
	$(SRCVM)/parsewav.o \
	$(SRCVM)/paw_datas.o \
	$(SRCVM)/paw_defs.o \
	$(SRCVM)/paw_dft.o \
	$(SRCVM)/paw_file.o \
	$(SRCVM)/paw_format.o \
	$(SRCVM)/paw_param.o \
	$(SRCVM)/paw_parse.o \
	$(SRCVM)/paw_parsecar.o \
	$(SRCVM)/paw_parsewav.o \
	$(SRCVM)/paw_util.o \
	$(SRCVM)/debugger_base.o \
	$(SRCVM)/debugger.o

FMGENOBJS:=$(SRCFMGEN)/fmgen.o \
	$(SRCFMGEN)/fmtimer.o \
	$(SRCFMGEN)/opm.o \
	$(SRCFMGEN)/opna.o \
	$(SRCFMGEN)/psg.o
#

DEPOBJS:=$(SRCDEP)/bml3mk5.o \
	$(SRCDEP)/cmt.o \
	$(SRCDEP)/comm.o \
	$(SRCDEP)/display.o \
	$(SRCDEP)/kanji.o \
	$(SRCDEP)/keyboard.o \
	$(SRCDEP)/memory.o \
	$(SRCDEP)/printer.o \
	$(SRCDEP)/sound.o \
	$(SRCDEP)/floppy.o \
	$(SRCDEP)/psgc.o \
	$(SRCDEP)/psg9c.o \
	$(SRCDEP)/board.o \
	$(SRCDEP)/registers.o \
	$(SRCDEP)/keyrecord.o \
	$(SRCDEP)/rtc.o \
	$(SRCDEP)/l3basic.o \
	$(SRCDEP)/timer.o 

GUIOBJSWX:=$(SRCGUIWX)/wx_dlg.o \
	$(SRCGUIWX)/wx_file_dlg.o \
	$(SRCGUIWX)/wx_volume_dlg.o \
	$(SRCGUIWX)/wx_config_dlg.o \
	$(SRCGUIWX)/wx_keybind_dlg.o \
	$(SRCGUIWX)/wx_recvid_dlg.o \
	$(SRCGUIWX)/wx_recaud_dlg.o \
	$(SRCGUIWX)/wx_seldrv_dlg.o \
	$(SRCGUIWX)/wx_ledbox.o \
	$(SRCGUIWX)/wx_vkeyboard.o \
	$(SRCGUIWX)/wx_about_dlg.o \
	$(SRCGUIWX)/wx_gui.o


VIDOBJSWAV:=$(SRCVIDWAV)/wav_rec_audio.o
VIDOBJSQTKIT:=$(SRCVIDQTKIT)/qt_rec_video.o
VIDOBJSAVKIT:=$(SRCVIDAVKIT)/avk_rec_common.o $(SRCVIDAVKIT)/avk_rec_audio.o $(SRCVIDAVKIT)/avk_rec_video.o
VIDOBJSFFM:=$(SRCVIDFFM)/ffm_loadlib.o $(SRCVIDFFM)/ffm_rec_audio.o $(SRCVIDFFM)/ffm_rec_video.o
# VIDOBJSWX:=$(SRCVIDWX)/wx_bitmap.o

VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSQTKIT) $(VIDOBJSAVKIT) $(VIDOBJSFFM) $(VIDOBJSWX) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o

ifeq ($(GUI_TYPE),GUI_TYPE_WXWIDGETS)
	GUIOBJS:=$(GUIOBJSWX)
endif

GUIOBJS:=$(GUIOBJS) $(SRCDIR)/gui/gui_base.o \
	$(SRCDIR)/gui/gui_keybinddata.o

OBJS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(EMUDEPOBJS) $(GUIOBJS) $(VIDOBJS)
DEPS_BASE:=$(OBJS_BASE:%.o=%.d)
OBJS:=$(OBJS_BASE:%=$(BUILDDIR)/%)
DEPS:=$(DEPS_BASE:%=$(BUILDDIR)/%)

MAKEFILEDEP:=$(BUILDDIR)/Makefile.dep

#
#
#

all: exe

exe: $(BUILDDIR) $(EXE)

depend: $(MAKEFILEDEP)

$(MAKEFILEDEP): $(BUILDDIR) $(DEPS)
	cat $(DEPS) > $@

$(EXE): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CDEFS) $(ARCH) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.cpp
	-$(CXX) $(CDEFS) $(CXXFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CDEFS) $(ARCH) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.c
	-$(CC) $(CDEFS) $(CFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.mm
	$(CXX) $(CDEFS) $(ARCH) $(CXXFLAGS) $(OBJC_CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.mm
	-$(CXX) $(CDEFS) $(CXXFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.m
	$(CC) $(CDEFS) $(ARCH) $(CFLAGS) $(OBJC_CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.m
	-$(CC) $(CDEFS) $(CFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

install: exe
	mkdir -p $(INSTALLDIR)/$(EXEDIR)
	cp -p $(EXE) $(INSTALLDIR)/$(EXEDIR)
	SetFile -t APPL $(INSTALLDIR)/$(EXEDIR)/$(EXEFILE)
#	cp -p $(DATADIR)/?*.* $(INSTALLDIR)/
	mkdir -p $(INSTALLDIR)/$(RESDIR)
	(cp -p $(SRCRES)/*.* $(INSTALLDIR)/$(RESDIR); exit 0)
	(cp -pR $(MACRESDIR)/ $(INSTALLDIR)/; exit 0)
	mkdir -p $(INSTALLDIR)/$(RESDIR)/$(LOCALEDIR)
	(cp -p $(SRCLOCALE)/*.xml $(INSTALLDIR)/$(RESDIR)/$(LOCALEDIR); exit 0)
	for i in $(SRCLOCALE)/*/*; do if [ -d $$i ]; then \
		mkdir -p $(INSTALLDIR)/$(RESDIR)/$$i; cp -p $$i/*.mo $(INSTALLDIR)/$(RESDIR)/$$i; \
	fi; done

$(BUILDDIR):
	mkdir -p $(BUILDDIR)/$(SRCFMGEN)
	mkdir -p $(BUILDDIR)/$(SRCDEP)
	mkdir -p $(BUILDDIR)/$(SRCOSDSDL)
	mkdir -p $(BUILDDIR)/$(SRCOSDMAC)
	mkdir -p $(BUILDDIR)/$(SRCOSDWX)
	mkdir -p $(BUILDDIR)/$(SRCGUIWX)
	mkdir -p $(BUILDDIR)/$(SRCVIDWAV)
	mkdir -p $(BUILDDIR)/$(SRCVIDQTKIT)
	mkdir -p $(BUILDDIR)/$(SRCVIDAVKIT)
	mkdir -p $(BUILDDIR)/$(SRCVIDFFM)
	mkdir -p $(BUILDDIR)/$(SRCVIDWX)

clean:
	rm -rf $(BUILDDIR)

include $(MAKEFILEDEP)
