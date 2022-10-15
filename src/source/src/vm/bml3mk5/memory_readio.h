/** @file memory_readio.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.02.01 -

	@brief [ memory read io ]
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
				data = d_fdd->READ_IO8(addr);
			}
			break;
		case 0xeff08:
			// now halt (8inch fdd) (cannot read)
			break;
		case 0xeff0c:
			// type sel (8inch fdd)
			if (IOPORT_USE_5FDD) {
				data = d_fdd->READ_IO8(addr);
			}
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
				data = d_fdc3->READ_IO8(addr - 0xff18);

//				logging->out_debugf("fdc3r a:%04x d:%02x",addr,data);
			}
			break;
		case 0xff20:
			// fdd drive select
			if (IOPORT_USE_3FDD) {
				data = d_fdd->READ_IO8(0);

//				logging->out_debugf("fdd3r a:%04x d:%02x",addr,data);
			}
			break;
#endif
		case 0xff30:
		case 0xff31:
		case 0xff32:
		case 0xff33:
			// psg
			if (IOPORT_USE_PSG6) {
				data = d_psg3->READ_IO8(addr - 0xff30);
//				logging->out_debugf("psgr a:%04x d:%02x",addr,data);
			}
			break;
#ifdef USE_RTC
		case 0xff3a:
			// rtc read
			if (IOPORT_USE_RTC) {
				data = d_rtc->READ_IO8(addr & 3);
			}
			break;
#endif
		case 0xff3c:
		case 0xff3d:
		case 0xff3e:
		case 0xff3f:
			// extended LPT board
			if (IOPORT_USE_EXPIA) {
				data = d_pia_ex->READ_IO8(addr - 0xff3c);
			}
			break;

		case 0xff40:
		case 0xff41:
			// extended COM board
			if (IOPORT_USE_EXACIA) {
				data = d_acia_ex->READ_IO8(addr - 0xff40);
			}
			break;

		case 0xff60:
		case 0xff61:
		case 0xff62:
		case 0xff63:
			// extended PIA
			data = d_pia_ex2->READ_IO8(addr - 0xff60);
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
				data = d_psg9->READ_IO8(addr - 0xff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
			break;

		case 0xff75:
		case 0xff76:
			// psg9
			if (IOPORT_USE_PSG9) {
				data = d_psg9->READ_IO8(addr - 0xff70);
//				logging->out_debugf("psg9r a:%04x d:%02x",addr,data);
			}
			// kanji rom
			if (IOPORT_USE_KANJI) {
				data = d_kanji->READ_IO8(addr - 0xff75);
//				logging->out_debugf("kanjir a:%04x d:%02x",addr,data);
			}
			break;

		case 0xffc0:
		case 0xffc1:
		case 0xffc2:
		case 0xffc3:
			// pia
			data = d_pia->READ_IO8(addr - 0xffc0);
			break;

		case 0xffc4:
		case 0xffc5:
			// acia
			data = d_acia->READ_IO8(addr - 0xffc4);
			break;

		case 0xffc6:
		case 0xffc7:
			// crtc
			data = d_crtc->READ_IO8(addr - 0xffc6);
			break;

		case 0xffc8:
			// keyboard nmi
			data = d_key->READ_IO8(addr);
			break;
		case 0xffc9:
			// newon number
			data = config.dipswitch;
			break;
		case 0xffca:
			// timer irq
			data = d_timer->READ_IO8(addr);
			break;
		case 0xffcb:
			// light pen flag (cannot write)
			data = d_key->READ_IO8(addr);
			break;
		case 0xffd0:
			// mode select
#ifdef DEBUG_READ_OK
			data = ram[0xffd0];
#endif
			break;
		case 0xffd1:
			// trace counter
			break;
		case 0xffd2:
			// remote switch
#ifdef DEBUG_READ_OK
			data = REG_REMOTE;
#endif
			break;
		case 0xffd3:
			// music sel
#ifdef DEBUG_READ_OK
			data = ram[0xffd3];
#endif
			break;
		case 0xffd4:
			// time mask
#ifdef DEBUG_READ_OK
			data = ram[0xffd4];
#endif
			break;
		case 0xffd5:
			// light pen bl
#ifdef DEBUG_READ_OK
			data = ram[0xffd5];
#endif
			break;
		case 0xffd6:
			// interace sel
#ifdef DEBUG_READ_OK
			data = ram[0xffd6];
#endif
			break;
		case 0xffd7:
			// baud sel
#ifdef DEBUG_READ_OK
			data = REG_BAUD_SEL;
#endif
			break;
		case 0xffd8:
		case 0xffd9:
		case 0xffda:
		case 0xffdb:
		case 0xffdc:
		case 0xffdd:
		case 0xffde:
		case 0xffdf:
			// color register sel
#ifdef _DEBUG_CRAM
			logging->out_debugf("cr c%02x",color_reg);
#endif
			data = ((color_reg & 0xbf) | 0x40);
			break;
		case 0xffe0:
		case 0xffe1:
		case 0xffe2:
		case 0xffe3:
		case 0xffe4:
		case 0xffe5:
		case 0xffe6:
		case 0xffe7:
			// keyboard scan data
			data = d_key -> READ_IO8(0xffe0);
			break;
		case 0xffe8:
			// bank register
#ifdef DEBUG_READ_OK
			data = mem_bank_reg2;
#endif
			break;
		case 0xffe9:
			// IG mode register
			break;
		case 0xffea:
			// IG en register
			break;
	}
}
