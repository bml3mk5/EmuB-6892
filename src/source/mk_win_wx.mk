#
# for Windows + MinGW + MSYS
#
# export GUI_TYPE
# export BUILDDIR
# export LIBS
# export LDFLAGS
# export CDEFS
# export CFLAGS
# export CXXFLAGS
# export INSTALLDIR

CC:=gcc
CXX:=g++
LD:=g++

FFMPEGDIR:=/D/Devel/vc/ffmpeg-3.4.1-win64-dev/include

COMMONCFLAGS:=-I/usr/local/include -I/usr/include -I$(FFMPEGDIR) -I./include

CFLAGS:=$(CFLAGS) $(COMMONCFLAGS)
CXXFLAGS:=$(CXXFLAGS) $(COMMONCFLAGS)

EXEFILE:=bml3mk5

DATADIR:=data

SRCDIR:=src
SRCOSD:=$(SRCDIR)/osd
SRCOSDSDL:=$(SRCOSD)/SDL
SRCOSDWX:=$(SRCOSD)/wxwidgets
SRCOSDWIN:=$(SRCOSD)/windows
SRCVM:=$(SRCDIR)/vm
SRCFMGEN:=$(SRCVM)/fmgen
SRCDEP:=$(SRCVM)/bml3mk5
SRCVID:=$(SRCDIR)/video
SRCVIDWAV:=$(SRCVID)/wave
SRCVIDVFW:=$(SRCVID)/vfw
SRCVIDFFM:=$(SRCVID)/ffmpeg
SRCVIDWX:=$(SRCVID)/wxwidgets
SRCGUIWX:=$(SRCDIR)/gui/wxwidgets

RESDIR:=res
SRCRES:=$(SRCDIR)/$(RESDIR)/common

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
	$(SRCOSDWIN)/win_uart.o \
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
	$(SRCOSD)/vkeyboardbase.o \
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
VIDOBJSVFW:=$(SRCVIDVFW)/vfw_rec_video.o
VIDOBJSFFM:=$(SRCVIDFFM)/ffm_loadlib.o $(SRCVIDFFM)/ffm_rec_audio.o $(SRCVIDFFM)/ffm_rec_video.o

VIDOBJS:=$(VIDOBJSWAV) $(VIDOBJSVFW) $(VIDOBJSFFM) $(SRCVID)/rec_audio.o $(SRCVID)/rec_video.o

ifeq ($(GUI_TYPE),GUI_TYPE_WXWIDGETS)
	GUIOBJS:=$(GUIOBJSWX)
endif

GUIOBJS:=$(GUIOBJS) $(SRCDIR)/gui/gui_base.o \
	$(SRCDIR)/gui/gui_keybinddata.o

SRCWINRES:=$(SRCDIR)/$(RESDIR)/windows
WINRESOBJS:=$(SRCWINRES)/bml3mk5.res
ifeq ($(GUI_TYPE),GUI_TYPE_WINDOWS)
	WINRESOBJS:=$(WINRESOBJS) $(SRCWINRES)/bml3mk5_gui.res
endif

OBJS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(EMUDEPOBJS) $(GUIOBJS) $(VIDOBJS) $(WINRESOBJS)
DEPS_BASE:=$(DEPOBJS) $(VMOBJS) $(FMGENOBJS) $(EMUOBJS) $(EMUOSDOBJS) $(GUIOBJS) $(VIDOBJS) 
OBJS:=$(OBJS_BASE:%=$(BUILDDIR)/%)
DEPS:=$(DEPS_BASE:%.o=$(BUILDDIR)/%.d)

WINDRES:=windres.exe

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
	$(CXX) $(CDEFS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.cpp
	-$(CXX) $(CDEFS) $(CXXFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: %.c
	-$(CC) $(CDEFS) $(CFLAGS) -MM $< | sed 's#^.*\.o:#$@:#' | sed 's/\.d:/.o:/' > $@

$(BUILDDIR)/%.res: %.rc
	$(WINDRES) $< -O coff $(CDEFS) $(WX_CFLAGS) -o $@

install: exe
	mkdir -p $(INSTALLDIR)
	cp -p $(EXE) $(INSTALLDIR)
#	cp -p $(DATADIR)/?*.* $(INSTALLDIR)
	mkdir -p $(INSTALLDIR)/$(RESDIR)
	cp -p $(SRCRES)/*.* $(INSTALLDIR)/$(RESDIR)
	mkdir -p $(INSTALLDIR)/$(LOCALEDIR)
	cp -p $(SRCLOCALE)/*.xml $(INSTALLDIR)/$(LOCALEDIR)
	for i in $(LOCALEDIR)/*/*; do if [ -d $$i ]; then \
		mkdir -p $(INSTALLDIR)/$$i; cp -p $$i/*.mo $(INSTALLDIR)/$$i; \
	fi; done

$(BUILDDIR):
	mkdir -p $(BUILDDIR)/$(SRCFMGEN)
	mkdir -p $(BUILDDIR)/$(SRCDEP)
	mkdir -p $(BUILDDIR)/$(SRCOSDSDL)
	mkdir -p $(BUILDDIR)/$(SRCOSDWX)
	mkdir -p $(BUILDDIR)/$(SRCOSDWIN)
	mkdir -p $(BUILDDIR)/$(SRCGUIWX)
	mkdir -p $(BUILDDIR)/$(SRCVIDWAV)
	mkdir -p $(BUILDDIR)/$(SRCVIDVFW)
	mkdir -p $(BUILDDIR)/$(SRCVIDFFM)
	mkdir -p $(BUILDDIR)/$(SRCVIDWIN)
	mkdir -p $(BUILDDIR)/$(SRCWINRES)

clean:
	rm -rf $(BUILDDIR)

include $(MAKEFILEDEP)
