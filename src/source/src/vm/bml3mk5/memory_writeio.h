/** @file memory_writeio.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.02.01 -

	@brief [ memory write io ]
*/

{
	// memory mapped i/o
	switch(addr & 0xffff) {
#ifdef USE_FD1
		case 0xff00:
		case 0xff01:
		case 0xff02:
		case 0xff03:
			// fdc
		case 0xff04:
			// fdd drive select
			if (IOPORT_USE_5FDD) {
				d_fdd->WRITE_IO8(addr, data);
			}
			break;
		case 0xff08:
			// halt (8inch fdd)
			if (IOPORT_USE_5FDD) {
				d_fdd->WRITE_IO8(addr, data);
			}
			break;
		case 0xff0c:
			// type sel (cannot write)
			break;

		case 0xff18:
		case 0xff19:
		case 0xff1a:
		case 0xff1b:
		case 0xff1c:
		case 0xff1d:
		case 0xff1e:
		case 0xff1f:
			// fdc
			if (IOPORT_USE_3FDD) {
				d_fdc3->WRITE_IO8(addr - 0xff18, data);

//				logging->out_debugf("fdc3w a:%04x d:%02x",addr,data);
			}
			break;
		case 0xff20:
			// fdd drive select
			if (IOPORT_USE_3FDD) {
				d_fdd->WRITE_IO8(0, data);

//				logging->out_debugf("fdd3w a:%04x d:%02x",addr,data);
			}
			break;
#endif
		case 0xff30:
		case 0xff31:
		case 0xff32:
		case 0xff33:
			// psg
			if (IOPORT_USE_PSG6) {
				d_psg3->WRITE_IO8(addr - 0xff30, data);
//				logging->out_debugf("psgw a:%04x d:%02x",addr,data);
			}
			break;
#ifdef USE_RTC
		case 0xff38:
		case 0xff39:
			// rtc
			if (IOPORT_USE_RTC) {
				d_rtc->WRITE_IO8(addr & 3, data);
			}
			break;
#endif
		case 0xff3c:
		case 0xff3d:
		case 0xff3e:
		case 0xff3f:
			// extended LPT board
			if (IOPORT_USE_EXPIA) {
				d_pia_ex->WRITE_IO8(addr - 0xff3c, data);
			}
			break;

		case 0xff40:
			// extended COM board
			if (IOPORT_USE_EXACIA) {
				d_comm1->WRITE_IO8(addr - 0xff40, data);
			}
			// through
		case 0xff41:
			// extended COM board
			if (IOPORT_USE_EXACIA) {
				d_acia_ex->WRITE_IO8(addr - 0xff40, data);
			}
			break;

		case 0xff60:
		case 0xff61:
		case 0xff62:
		case 0xff63:
			// extended PIA
			d_pia_ex2->WRITE_IO8(addr - 0xff60, data);
			break;

		case 0xff70:
		case 0xff71:
		case 0xff72:
		case 0xff73:
		case 0xff74:
		case 0xff77:
		case 0xff78:
		case 0xff79:
		case 0xff7a:
		case 0xff7b:
		case 0xff7c:
		case 0xff7d:
		case 0xff7e:
		case 0xff7f:
			// psg9
			if (IOPORT_USE_PSG9) {
				d_psg9->WRITE_IO8(addr - 0xff70, data);
//				logging->out_debugf("psg9w a:%04x d:%02x",addr,data);
			}
			break;

		case 0xff75:
		case 0xff76:
			// psg9
			if (IOPORT_USE_PSG9) {
				d_psg9->WRITE_IO8(addr - 0xff70, data);
//				logging->out_debugf("psg9w a:%04x d:%02x",addr,data);
			}
			// kanji rom
			if (IOPORT_USE_KANJI) {
				d_kanji->WRITE_IO8(addr - 0xff75, data);
//				logging->out_debugf("kanjiw a:%04x d:%02x",addr,data);
			}
			break;

		case 0xffc0:
		case 0xffc1:
		case 0xffc2:
		case 0xffc3:
			// pia
			d_pia->WRITE_IO8(addr - 0xffc0, data);
			break;

		case 0xffc4:
			// acia
			d_comm->WRITE_IO8(addr - 0xffc4, data);
			// pass through
		case 0xffc5:
			// acia
			d_acia->WRITE_IO8(addr - 0xffc4, data);
			break;

		case 0xffc6:
		case 0xffc7:
			// crtc
			d_crtc->WRITE_IO8(addr - 0xffc6, data);
			break;

		case 0xffc8:
			// keyborad nmi (cannot write)
			break;
		case 0xffca:
			// timer irq
			break;
		case 0xffcb:
			// light pen flag (cannot write)
			break;
		case 0xffd0:
			// mode select
			REG_MODE_SEL = (data & 0xff);
			d_disp->WRITE_IO8(addr, data);
			d_comm->write_signal(COMM::SIG_COMM_RS, data, 0x20);
			break;
		case 0xffd1:
			// trace counter
			fetch_trace_counter();
			break;
		case 0xffd2:
			// remote switch
			d_cmt->write_signal(CMT::SIG_CMT_REMOTE, data, 0x80);
			break;
		case 0xffd3:
			// music sel
//			logging->out_debugf("ms %02x",data);
			d_sound->write_signal(SOUND::SIG_SOUND_ON, 1, 1);
			d_sound->write_signal(SOUND::SIG_SOUND_SIGNAL, data, 0x80);
			break;
		case 0xffd4:
			// time mask
			d_timer->WRITE_IO8(addr, data & 0x80);
			break;
		case 0xffd5:
			// light pen bl
//			logging->out_debugf(_T("lpengl: %02x"),data);
			d_key->WRITE_IO8(addr, data & 0x80);
			break;
		case 0xffd6:
			// interace sel
			d_disp->WRITE_IO8(addr, data & 0x08);
			break;
		case 0xffd7:
			// baud sel
			d_cmt->write_signal(CMT::SIG_CMT_BAUD_SEL, data, 0x01);
			break;
		case 0xffd8:
		case 0xffd9:
		case 0xffda:
		case 0xffdb:
		case 0xffdc:
		case 0xffdd:
		case 0xffde:
		case 0xffdf:
			// color register
#ifdef _DEBUG_CRAM
			logging->out_debugf("cw c%02x->%02x",color_reg,data);
#endif
			color_reg = data & 0xbf;
			break;
		case 0xffe0:
		case 0xffe1:
		case 0xffe2:
		case 0xffe3:
		case 0xffe4:
		case 0xffe5:
		case 0xffe6:
		case 0xffe7:
			// keyboard mode register
			d_key -> WRITE_IO8(0xffe0, data);
			break;
		case 0xffe8:
			// bank register
			mem_bank_reg2 = data & 0xff;
			change_l3_memory_bank();
			break;
		case 0xffe9:
			// IG mode register
			set_igmode(0x100 | data);
			break;
		case 0xffea:
			// IG en register
			ig_enreg = data & 0x07;
			break;
	}
}
