/** @file cmt.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.13 -

	@brief [ cmt ]
*/

#include <math.h>
#include <stdlib.h>
#include "cmt.h"
//#include "../../emu.h"
#include "../acia.h"
#include "registers.h"
#include "../vm.h"
#include "../../logging.h"
#include "../../config.h"
#include "../../utility.h"
#include "../../fileio.h"
#include "../parsewav.h"

#define TAPE_WAV_FREQ	4800

#define T9X_IDENTIFIER "eMB-689X CassetteTapeImageFile  "

static const _TCHAR *support_ext[6]={_T(".wav"), _T(".l3c"), _T(".l3b"), _T(".l3"), _T(".t9x"), NULL};

void CMT::initialize()
{
	// data recorder
	fio = new FILEIO();
	wav = new PARSEWAV::ParseWav();
	memset(data_buffer, 0x00, sizeof(data_buffer));
	memset(c_onedata, 0x00, sizeof(c_onedata));
	data_read_pos = 0;
	data_write_pos = 0;
	data_read_size = 0;
	data_write_size = 0;
	data_total_pos = 0;
	file_type = 3;
	is_bytedata = true;
	need_header = false;
	data_len = 0;

	t9x_data = 0;
	t9x_dlen = 0;

	buffer_overflow = false;
	play = rec = false;
	REG_REMOTE = 0;
	remote_prev = 0;
	send_ok = true;
	recv_ok = false;
	fast_rotate = 0;
	file_startpos = 0;
	tmp_playrec = false;
	txdata_received = false;
	txdata = 0;

	REG_BAUD_SEL = 0;

	register_id[0] = -1;
	register_id[1] = -1;

//	relay_wav_enable = 0;
	relay_wav_loaded_at_first = false;

#if 0
	relay_data[0] = NULL;
	relay_size[0] = 0;
	relay_data[1] = NULL;
	relay_size[1] = 0;
	relay_play_pos = 0;
	relay_play = 0;
	UTILITY::tcscpy(relay_file[0], sizeof(relay_file[0]) / sizeof(relay_file[0][0]), _T("relay_on.wav"));
	UTILITY::tcscpy(relay_file[1], sizeof(relay_file[1]) / sizeof(relay_file[1][0]), _T("relayoff.wav"));
#endif
	m_relay_play = WAV_RELAY_ON;
	m_relay[WAV_RELAY_ON].set_file_name(_T("relay_on.wav"));
	m_relay[WAV_RELAY_OFF].set_file_name(_T("relayoff.wav"));

//	tape_wave_buf = NULL;
//	tape_wave_buf_wpos = 0;
//	tape_wave_buf_rpos = 0;

	set_baud(REG_BAUD_SEL);
}

void CMT::reset()
{
	load_relay_wav();

	REG_BAUD_SEL = 0;
	set_baud(REG_BAUD_SEL);
	for(int i=0; i<2; i++) {
		if (register_id[i] != -1) {
			cancel_my_event(i);
			register_my_event(i);
		}
	}
	REG_REMOTE = (pConfig->now_power_off ? 0 : 0x80);
}

void CMT::release()
{
	close_datarec();
	cancel_my_event(0);
	cancel_my_event(1);

//	delete[] tape_wave_buf;

//	delete[] relay_data[1];
//	delete[] relay_data[0];

	delete wav;
	delete fio;
}

void CMT::load_relay_wav()
{
	// load relay wav file
	const _TCHAR *app_path, *rom_path[2];
//	_TCHAR file_path[_MAX_PATH];
//	wav_header_t	 head;
//	wav_fmt_chank_t  fmt;
//	wav_data_chank_t data;
//	size_t			 data_len;

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = vm->application_path();

	for (int j=0; j<WAV_RELAY_ALL; j++) {
#if 0
		if ((relay_wav_enable & (1 << j)) == 0) {
			relay_data[j] = new uint8_t[sample_rate >> 2];	// 0.25sec
			memset(relay_data[j], 128, sizeof(uint8_t) * (sample_rate >> 2));
		}
#endif
		m_relay[j].alloc(sample_rate >> 2);	// 0.25sec
	}

//	FILEIO* fre = new FILEIO();
	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		for (int j=0; j<WAV_RELAY_ALL; j++) {
#if 0
			if ((relay_wav_enable & (1 << j)) == 0) {
				UTILITY::stprintf(file_path, _MAX_PATH, _T("%s%s"), app_path, relay_file[j]);
				if(fre->Fopen(file_path, FILEIO::READ_BINARY)) {
					PARSEWAV::Util util;
					if (util.CheckWavFormat(*fre, &head, &fmt, &data, &data_len) >= 0) {
						relay_size[j] = (int)util.ReadWavData(*fre, &fmt, data_len, relay_data[j], sample_rate, 8, sample_rate >> 2);
						relay_wav_enable |= (1 << j);
						logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, relay_file[j]);
					}
					fre->Fclose();
				}
			}
#endif
			if (m_relay[j].load_wav_file(app_path, sample_rate) > 0) {
				logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, m_relay[j].get_file_name());
			}
		}
	}
//	delete fre;

	for (int j=0; j<WAV_RELAY_ALL; j++) {
#if 0
		if ((relay_wav_enable & (1 << j)) == 0) {
			relay_size[j] = 0;
			delete[] relay_data[j];
			relay_data[j]=NULL;
			if (!relay_wav_loaded_at_first) {
				logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, relay_file[j]);
			}
		}
#endif
		if (!m_relay[j].is_enable()) {
			m_relay[j].clear();
			if (!relay_wav_loaded_at_first) {
				logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, m_relay[j].get_file_name());
			}
		}

	}
	relay_wav_loaded_at_first = true;
}

void CMT::write_io8(uint32_t addr, uint32_t data)
{
//	set_data(data);
}

void CMT::ready_data(uint32_t data)
{
	txdata = data;
	txdata_received = true;
}

void CMT::set_data(uint32_t data)
{
	int len, c_len;

	if (REG_REMOTE && recv_ok && rec) {
		if (pConfig->NowRealModeDataRec()) {
			set_tape_wave(data);
		}
		if (data_write_pos >= CMTDATA_BUFFER_SIZE) {
			buffer_overflow = true;
			if (d_ctrl != NULL) {
				d_ctrl->write_signal(ACIA::SIG_ACIA_ERROR, 1, 1);
			}
		} else {
			switch (file_type) {
			case 0:
				// .wav
				if (need_header) {
					// wav header
					wav->SetSampleRatePos(pConfig->wav_sample_rate);
					wav->SetSampleBitsPos(pConfig->wav_sample_bits);
					//
					data_write_pos = wav->InitFileHeader(data_buffer);
					data_len = data_write_pos;
					file_startpos = data_write_pos;
					need_header = false;
				}
				len = wav->SetData(data, data_len, &data_buffer[data_write_pos], (int)sizeof(data_buffer) - data_write_pos, &c_len);
				data_write_pos+=len;
				data_len+=len;

				data_total_pos+=c_len;
				break;
			case 1:
				// .l3c
				len = wav->SetData(data, data_len, &data_buffer[data_write_pos], (int)sizeof(data_buffer) - data_write_pos, &c_len);
				data_write_pos+=len;
				data_len+=len;

				data_total_pos+=c_len;
				break;
			case 2:
				// .l3b
				data = (data & 1) | 0x30;
				if (data_len >= 110
				&& (data_buffer[data_write_pos - 1] & 1) == 1
				&& (data & 1) == 0) {
					// cr + lf
					data_buffer[data_write_pos]='\r';
					data_write_pos++;
					data_buffer[data_write_pos]='\n';
					data_write_pos++;
					data_len = 0;
				}
				data_buffer[data_write_pos]=data & 0xff;
				data_write_pos++;
				data_len++;

				data_total_pos += 8;
				break;
			case 4:
				// .t9x
				if (need_header) {
					// set dummy header
					memset(data_buffer, 0, sizeof(uint8_t) * 64);
					data_write_pos = 64;
					data_len = data_write_pos;
					file_startpos = data_write_pos;
					need_header = false;
				}
				t9x_data |= ((data & 1) << (t9x_dlen));
				logging->out_debugf(_T("t9x  w: %d-%d %d"), data_write_pos, t9x_dlen, data);
				t9x_dlen++;
				if (t9x_dlen >= 8) {
					data_buffer[data_write_pos]=t9x_data;
					logging->out_debugf(_T("t9x wt: %d-%d %02x"), data_write_pos, t9x_dlen, t9x_data);
					data_write_pos++;
					data_len++;
					t9x_data = 0;
					t9x_dlen = 0;
				}
				data_total_pos += 8;
				break;
			default:
				// .l3
				data_buffer[data_write_pos]=data & 0xff;
				data_write_pos++;
				data_len++;

				data_total_pos += 88;
				break;
			}
		}
	}
}

uint32_t CMT::read_io8(uint32_t addr)
{
	return get_data();
}

uint32_t CMT::get_data()
{
	uint32_t data = 0;
	static int cnt = 0;
	bool cont = false;

	// read 1byte from buffer
	if (REG_REMOTE) {
		do {
			cont = false;

			// read from file
			if (data_read_pos == data_read_size) {
				if (file_type >= 2) {
					load_image(0);
				} else {
					load_wav_image();
				}
			}
			if (data_read_size == 0) break;

			switch(file_type) {
			case 0:
				// .wav

				// get 1 data
				data = data_buffer[data_read_pos];
				data_read_pos++;

				if (pConfig->NowRealModeDataRec()) {
					set_tape_direct((int16_t *)w_onedata, w_onelen >> 1);
				}
				data_total_pos = ((int)wav->GetSamplePos() / samples_base);
				break;
			case 1:
				// .l3c

				// get 1 data
				data = data_buffer[data_read_pos];
				data_read_pos++;

				if (pConfig->NowRealModeDataRec()) {
					set_tape_amp(c_onedata, c_onelen);
				}
				data_total_pos = (int)wav->GetSamplePos();
				break;
			case 2:
				// .l3b

				// get 1 data
				data = data_buffer[data_read_pos];
				data_read_pos++;

				if ((data & 0xff) != 0x0a && (data & 0xff) != 0x0d) {
					if (pConfig->NowRealModeDataRec()) {
						set_tape_wave(data);
					}
					data_total_pos += 8;
				} else {
					cont = true;
				}
				break;
			case 4:
				// .t9x
				if (t9x_dlen >= 8) {
					// get 1 data
					t9x_data = data_buffer[data_read_pos];
					data_read_pos++;
					t9x_dlen = 0;
				}
				data = ((t9x_data >> t9x_dlen) & 1);
//				logging->out_debugf(_T("t9x r: %d-%d %d"), data_read_pos, t9x_dlen, data);
				t9x_dlen++;

				if (pConfig->NowRealModeDataRec()) {
					set_tape_wave(data);
				}
				data_total_pos += 8;
				break;
			default:
				// .l3 file

				// get 1 data
				data = data_buffer[data_read_pos];
				data_read_pos++;

				if (pConfig->NowRealModeDataRec()) {
					set_tape_wave(data);
				}
				data_total_pos += 88;
				break;
			}
		} while (cont && data_read_pos <= data_read_size);
	} else {
		data = cnt ? 0xff : 0;
		cnt--;
		if (cnt < 0) cnt = 15;
	}
	return data;
}

uint32_t CMT::get_data_fast()
{
	int pos = 0;
	uint32_t data = 0;
	bool cont = false;

	// read 1byte from buffer
	do {
		cont = false;

		if (file_type >= 2) {
			if (data_read_pos == data_read_size) {
				pos = load_image(fast_rotate);
			} else {
				pos = 1;
			}
			if (data_read_size == 0) break;
		}

		// read from file
		switch(file_type) {
		case 0:
			// .wav
			pos = load_wav_image();
			data_total_pos = ((int)wav->GetSamplePos() / samples_base);
			break;
		case 1:
			// .l3c
			pos = load_wav_image();
			data_total_pos = (int)wav->GetSamplePos();
			break;
		case 2:
			// .l3b

			// get 1 data
			data = data_buffer[data_read_pos];
			data_read_pos++;

			if ((data & 0xff) != 0x0a && (data & 0xff) != 0x0d) {
				data_total_pos += (8 * fast_rotate);
			} else {
				cont = true;
			}
			break;
		case 4:
			// .t9x
			if (t9x_dlen < 0 || t9x_dlen >= 8) {
				// get 1 data
				t9x_data = data_buffer[data_read_pos];
				data_read_pos++;
				t9x_dlen = (fast_rotate >= 0 ? 0 : 7);
			}
			data = ((t9x_data >> t9x_dlen) & 1);
			t9x_dlen += fast_rotate;
			data_total_pos += (8 * fast_rotate);
			break;
		default:
			// .l3 file

			// get 1 data
			data = data_buffer[data_read_pos];
			data_read_pos++;

			data_total_pos += (88 * fast_rotate);
			break;
		}
	} while (cont && data_read_pos <= data_read_size);

	if (pos == 0) {
		// end of data
		cancel_my_event(1);
		fast_rotate = 0;
		wav->SetStartPhase(fast_rotate);
		if (tmp_playrec) {
			register_my_event(0);
		}
		tmp_playrec = false;

//		if (data_total_pos > 0) {
//			// if file is not top position, will not output header.
//			need_header = false;
//		}

		// refresh file buffer
		fio->Fseek(0, FILEIO::SEEKCUR);
	}
	return data;
}

void CMT::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_CMT_REMOTE:
			// remote on/off
			remote_prev = REG_REMOTE;
			REG_REMOTE = (data & mask) ? 0x80 : 0;
			break;
		case SIG_CMT_BAUD_SEL:
			// baud select
			REG_BAUD_SEL = (data & mask);
			set_baud(REG_BAUD_SEL);
			if (register_id[0] != -1) {
				cancel_my_event(0);
				register_my_event(0);
			}
			break;
		case ACIA::SIG_ACIA_DTR:
			send_ok = (data & mask) ? true : false;
			break;
		case ACIA::SIG_ACIA_RTS:
			recv_ok = (data & mask) ? true : false;
			break;
		case ACIA::SIG_ACIA_TXDATA:
			// receive data from acia
			ready_data(data);
//			set_data(data);
			break;
		case SIG_CPU_RESET:
			now_reset = (data & mask) ? true : false;
			if (now_reset) {
				reset();
			}
			break;
	}

}

void CMT::set_baud(int sel)
{
	switch(sel & 1) {
	case 1:
		// 1200baud
		baud_rate = 1200;
		tape_wave_nums = 2;
		break;
	default:
		// 600baud
		baud_rate = 600;
		tape_wave_nums = 4;
	}
	wav->SetBaudAndFrequency(baud_rate, sel);
}

bool CMT::play_datarec(const _TCHAR* filename)
{
	close_datarec();

	if(fio->Fopen(filename, FILEIO::READ_BINARY)) {
		check_extension(filename);

		// load image file
		play = true;
		cancel_my_event(0);
		register_my_event(0);

		play = set_load_param();
//		if (play) load_image();

	}
	return play;
}

/// Rewind
void CMT::rewind_datarec()
{
	if (play || rec) {
		fast_rotate = -1;
		wav->SetStartPhase(fast_rotate);
		wav->ClearData();
		tmp_playrec = tmp_playrec || (register_id[0] != -1);
		cancel_my_event(0);
		register_my_event(1);
	}
}

/// Fast Forward
void CMT::fast_forward_datarec()
{
	if (play || rec) {
		fast_rotate = 1;
		wav->SetStartPhase(fast_rotate);
		wav->ClearData();
		tmp_playrec = tmp_playrec || (register_id[0] != -1);
		cancel_my_event(0);
		register_my_event(1);
	}
}

void CMT::stop_datarec()
{
	if (fast_rotate) {
		cancel_my_event(1);
		fast_rotate = 0;
		wav->SetStartPhase(fast_rotate);
		if (tmp_playrec) {
			register_my_event(0);
		}
		tmp_playrec = false;
	}
}

void CMT::realmode_datarec()
{
	// toggle
	pConfig->SetRealModeDataRec(!pConfig->NowRealModeDataRec());
	if (register_id[0] != -1) {
		cancel_my_event(0);
		register_my_event(0);
	}
}

bool CMT::datarec_opened(bool play_mode)
{
	if (play_mode) {
		return play;
	} else {
		return rec;
	}
}

bool CMT::rec_datarec(const _TCHAR* filename)
{
	close_datarec();

	if(!(fio->Fopen(filename, FILEIO::READ_WRITE_BINARY)
	  || fio->Fopen(filename, FILEIO::WRITE_BINARY))) {
		// open failed
		return false;
	}
	fio->Fclose();
	wav->CloseFile();
	if(fio->Fopen(filename, FILEIO::READ_WRITE_BINARY))	{
		check_extension(filename);

		data_write_pos = 0;
		data_len = 0;
		rec = true;

		cancel_my_event(0);
		register_my_event(0);

		set_save_param();

		data_total_pos = 0;
	}

	return rec;
}

void CMT::close_datarec()
{
	// close file
	if(rec) {
		save_image();
		if (file_type == 0) {
			// .wav file
			wav->SetFileHeader(fio);
		} else if (file_type == 4) {
			// .t9x
			set_file_header(fio, file_type);
		}
	}

	if(play || rec) {
		cancel_my_event(0);
		fio->Fclose();
		wav->CloseFile();
	}
	cancel_my_event(1);
	fast_rotate = 0;
	wav->SetStartPhase(fast_rotate);
	tmp_playrec = false;

	play = rec = false;
}

bool CMT::set_load_param()
{
	if (file_type <= 1) {
		// .l3c or .wav
		if (wav->OpenFile(*fio, file_type) != 0) {
			logging->out_log(LOG_ERROR, wav->Errmsg());
			return false;
		}
		wav->SetParseParamerers(pConfig->wav_half, pConfig->wav_reverse, pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0, pConfig->wav_correct_amp[0], pConfig->wav_correct_amp[1]);
		wav->InitData(*fio, baud_rate, sizeof(w_onedata));
	} else {
		// .l3b or .l3 or .t9x
		if (check_file_format(fio, file_type) != 0) {
			logging->out_logf(LOG_ERROR,_T("This is not %s format file."),support_ext[file_type]);
			return false;
		}
	}
	data_read_pos = 0;
	data_read_size = 0;

	t9x_data = 0;
	t9x_dlen = 8;

	data_total_pos = 0;

	return true;
}

bool CMT::set_save_param()
{
	if (file_type <= 1) {
		// .l3c or .wav
		int rc = wav->OpenFile(*fio, file_type, pConfig->wav_sample_rate, pConfig->wav_sample_bits, 1);
		if (rc > 1) {
			logging->out_log(LOG_ERROR, wav->Errmsg());
			return false;
		}
		if (rc == 0) {
			// get sample_rate and sample_bits parameters from file data.
			pConfig->wav_sample_rate = wav->GetSampleRatePos();
			pConfig->wav_sample_bits = wav->GetSampleBitsPos();
			need_header = false;
		} else {
			// new file
			wav->SetSampleRatePos(pConfig->wav_sample_rate);
			wav->SetSampleBitsPos(pConfig->wav_sample_bits);
			need_header = true;
		}
		wav->InitData(*fio, baud_rate, sizeof(w_onedata));
	} else {
		// .l3b or .l3 or .t9x
		int rc = check_file_format(fio, file_type);
		if (rc > 1) {
			logging->out_logf(LOG_ERROR,_T("This is not %s format file."),support_ext[file_type]);
			return false;
		}

		t9x_data = 0;
		t9x_dlen = 0;
		need_header = true;
	}
	data_read_pos = data_write_pos;
	data_read_size = data_write_pos;

	return true;
}

/// load tape image from file
int CMT::load_wav_image()
{
	int pos = 0;
	memset(data_buffer, 0x00, sizeof(data_buffer));
	// .l3c or .wav
	w_onedata[0]='\0';
	w_onelen = sizeof(w_onedata);
	c_onedata[0]='\0';
	c_onelen = sizeof(c_onedata);
	data_buffer[0]='\0';
	pos = wav->GetData(fast_rotate, data_buffer, data_read_size, w_onedata, &w_onelen, c_onedata, &c_onelen);
//		data_read_size = (data_read_size >= 0 ? data_read_size : 0);
	data_read_pos = 0;
	data_write_pos = 0;
	data_len = 0;
	return pos;
}

/// load tape image from file
/// @param[in] dir 1 or -1
int CMT::load_image(int dir)
{
	int pos = 0;
	memset(data_buffer, 0x00, 2 /*sizeof(data_buffer)*/);
	// .l3b or .l3 or .t9x
	if (dir >= 0) {
		data_read_size = (int)fio->Fread(data_buffer, sizeof(uint8_t), 1 /*sizeof(data_buffer)*/);
	} else {
		uint32_t nowpos = fio->Ftell();
		if (nowpos <= file_startpos) {
			fio->Fseek(file_startpos, FILEIO::SEEKSET);
			data_read_size = 0;
		} else {
			if (fio->Fseek(dir, FILEIO::SEEKCUR) != 0) {
				data_read_size = 0;
			} else {
				data_read_size = (int)fio->Fread(data_buffer, sizeof(uint8_t), 1 /*sizeof(data_buffer)*/);
				fio->Fseek(dir, FILEIO::SEEKCUR);
			}
		}
	}
	data_read_pos = 0;
	data_write_pos = 0;
	data_len = 0;
	pos = data_read_size;
	return pos;
}

void CMT::save_image()
{
	// binary image
	if (data_write_pos > 0) {
		fio->Fwrite(data_buffer, data_write_pos, 1);
		data_write_pos = 0;
	}
}

int CMT::check_extension(const _TCHAR* filename)
{
	int i = 0;
	file_type = -1;
	is_bytedata = false;
	while (support_ext[i] != NULL) {
		if (UTILITY::check_file_extension(filename, support_ext[i])) {
			file_type = i;
			break;
		}
		i++;
	}
	if (file_type < 0) {
		// default = .l3
		file_type = 3;
	}
	if (file_type == 3) {
		is_bytedata = true;
	}
	return file_type;
}

// ----------------------------------------------------------------------------
/// @return 0:exist file  1:no file(new file) 
int CMT::check_file_format(FILEIO *fp, int file_type)
{
	int rc = 0;
	switch(file_type) {
	case 4:
		// .t9x
		file_startpos = 0;
		if (fp->Fread(&t9x_header, sizeof(t9x_header), 1) != 1) {
			rc = 1;
			break;
		}
		if (memcmp(t9x_header.ident, T9X_IDENTIFIER, 32) != 0) {
			rc = 1;
			break;
		}
		file_startpos = fio->Ftell();
		break;
	default:
		file_startpos = 0;
		break;
	}
	return rc;
}

int CMT::set_file_header(FILEIO *fp, int file_type)
{
	int rc = 0;
	switch(file_type) {
	case 4:
		// .t9x
		{
			t9x_header_t hed;
			// if data remain, flush it
			if (t9x_dlen > 0) {
				fp->Fputc(t9x_data);
			}
			//
			memset(&hed, 0, sizeof(hed));
			UTILITY::strcpy(hed.ident, sizeof(hed.ident), T9X_IDENTIFIER);
			hed.data1=0;
			hed.data2=9;	// TODO: unknown number?
			fp->Fseek(0, FILEIO::SEEKSET);
			fp->Fwrite(&hed, sizeof(hed), 1);
		}
		break;
	default:
		break;
	}
	return rc;
}

// ----------------------------------------------------------------------------
void CMT::register_my_event(int event_id)
{
	int period = 1;

	if(register_id[event_id] != -1) return;

	switch(event_id) {
	case 0:
		if (pConfig->NowRealModeDataRec()) {
			if (is_bytedata) {
				// read/write 1byte data per one process
				period = (int)(CPU_CLOCKS / baud_rate * 11);	// 1startbit + 8databit + 2stopbit
			} else {
				// read/write 1bit data per one process
				period = (int)(CPU_CLOCKS / baud_rate);
			}
		} else {
			period = (int)(CPU_CLOCKS / 38400);
		}
		register_event_by_clock(this, event_id, period, true, &register_id[event_id]);
		break;
	case 1:
		period = (int)(CPU_CLOCKS / (file_type != 3 ? 38400 : 4800));
//		period = 4 * ((file_type != 3) ? 1 : 11);
		register_event_by_clock(this, event_id, period, true, &register_id[event_id]);
		break;
	}
}

void CMT::cancel_my_event(int event_id)
{
	if(register_id[event_id] != -1) {
		cancel_event(this, register_id[event_id]);
	}
	register_id[event_id] = -1;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void CMT::event_callback(int event_id, int err)
{
	uint32_t data = 0;
	uint32_t mask = 0;

	switch(event_id) {
	case 0:
		if(REG_REMOTE) {
			// send to acia
			if (d_ctrl != NULL) {
				// send to rxclk
				send_ok = false;
				d_ctrl->write_signal(ACIA::SIG_ACIA_RXCLK, 1, 1);

				if (play && (pConfig->NowRealModeDataRec() || send_ok || now_reset)) {
					// read data
					data = get_data();

					if (is_bytedata) {
						// .l3
						mask = 0xff;
					} else {
						mask = 1;
					}
					// send data to acia
					d_ctrl->write_signal(ACIA::SIG_ACIA_RXDATA, data, mask);
				}
			}
			// receive from acia
			// send to txclk then acia send data to me
			if (d_ctrl != NULL) {
				if (is_bytedata) {
					// .l3
					mask = 0xff;
				} else {
					mask = 1;
				}
				txdata_received = false;
				txdata = 0xff;
				d_ctrl->write_signal(ACIA::SIG_ACIA_TXCLK, 1, mask);
			}
			if (rec) {
				// set data to buffer
				if (txdata_received || pConfig->NowRealModeDataRec()) {
					set_data(txdata);
				}
				// write data to file
				if (data_write_pos > 11) {
					save_image();
				}
			}
		} else {
			if (rec) {
				save_image();
			}
		}
		break;
	case 1:
		// fast forward/rewind
		get_data_fast();
		break;
	}
//	update_event();
}

// ----------------------------------------------------------------------------
void CMT::mix(int32_t* buffer, int cnt)
{
//	int32_t *buffer_orig = buffer;

	// relay wave sound
	if (REG_REMOTE != 0 && remote_prev == 0) {
		// relay on sound
//		m_relay[WAV_RELAY_OFF].stop();
		m_relay[WAV_RELAY_ON].play();
		m_relay_play = WAV_RELAY_ON;
//		relay_play_pos = 0;
	} else if (REG_REMOTE == 0 && remote_prev != 0) {
		// relay off sound
//		m_relay[WAV_RELAY_ON].stop();
		m_relay[WAV_RELAY_OFF].play();
		m_relay_play = WAV_RELAY_OFF;
//		relay_play_pos = 0;
	}
	remote_prev = REG_REMOTE;

#if 0
	if (relay_play) {
		if (relay_volume) {
			for(int i=0; i<cnt; i++) {
				int32_t sample = ((int)relay_data[relay_play-1][relay_play_pos] - 128) * relay_volume / 128;
				*buffer++ += sample;
				*buffer++ += sample;
				relay_play_pos++;
				if (relay_play_pos >= relay_size[relay_play-1]) {
					break;
				}
			}
		}
		if (relay_play_pos >= relay_size[relay_play-1]) {
			relay_play = 0;
		}
	}
#endif
	m_relay[m_relay_play].mix(buffer, cnt);

	// tape wave sound
	m_tape_wave.mix(buffer, cnt);
#if 0
//	if (tape_wave_buf_wpos > tape_wave_buf_rpos) {
		buffer = buffer_orig;
		for(int i=0; i<cnt; i++) {
			*buffer++ += tape_wave_buf[tape_wave_buf_rpos];
			*buffer++ += tape_wave_buf[tape_wave_buf_rpos];
			tape_wave_buf_rpos++;
			if (tape_wave_buf_wpos <= tape_wave_buf_rpos) {
				break;
			}
		}
		// shift
		if (tape_wave_buf_wpos > (TAPEWAV_BUFFER_SIZE/2)) {
			for(int pos=tape_wave_buf_rpos; pos<tape_wave_buf_wpos; pos++) {
				tape_wave_buf[pos-tape_wave_buf_rpos] = tape_wave_buf[pos];
			}
			tape_wave_buf_wpos -= tape_wave_buf_rpos;
			tape_wave_buf_rpos = 0;
		}
//	} else {
//		surplus = 0;
	}
#endif
}

void CMT::set_volume(int decibel_relay, int decibel_tape, bool mute_relay, bool mute_tape)
{
	int relay_volume = (int)(16384.0 * pow(10.0, decibel_relay / 40.0));
	if (mute_relay) relay_volume = 0;
	for(int j=0; j<WAV_RELAY_ALL; j++) {
		m_relay[j].set_volume(relay_volume, relay_volume);
	}

	int tape_volume = (int)(16384.0 * pow(10.0, decibel_tape / 40.0));
	if (mute_tape) tape_volume = 0;
	m_tape_wave.set_volume(tape_volume, tape_volume);
}

void CMT::initialize_sound(int rate, int decibel)
{
	sample_rate_base = sample_rate = rate;
//	max_vol = volume;
	set_volume(decibel, decibel, false, false);
//	relay_volume = 0;
//	tape_volume = 0;

	// load relay wav file
	load_relay_wav();

	// create tape wave
	create_tape_wave();

	// set rate for speaker out 	
	wav->SetFormatForSound(sample_rate, 16, 1);
	samples_base = sample_rate / 4800;
}

// ----------------------------------------------------------------------------
void CMT::create_tape_wave()
{
//	tape_wave_buf = new int16_t[TAPEWAV_BUFFER_SIZE + 1];
	m_tape_wave.alloc(TAPEWAV_BUFFER_SIZE);
	m_tape_wave.play();

	devide = 0;
	surplus = 0;
	a_prev = 0;

	tape_wave_onoff = 1;

	tape_wav_freq = TAPE_WAV_FREQ;
}

void CMT::set_tape_wave(uint32_t data)
{
	if (!is_bytedata) {
		// 1bit data
		if (data & 1) {
			// 2400Hz * 4
			set_tape_one_wave(tape_wave_nums << 1, 1);
		} else {
			// 1200Hz * 2
			set_tape_one_wave(tape_wave_nums, 2);
		}
	} else {
		// 1byte data
		// startbit
		// 1200Hz * 2
		set_tape_one_wave(tape_wave_nums, 2);
		// databit
		for(int b=0; b<8; b++) {
			if (data & (1 << b)) {
				// 2400Hz * 4
				set_tape_one_wave(tape_wave_nums << 1, 1);
			} else {
				// 1200Hz * 2
				set_tape_one_wave(tape_wave_nums, 2);
			}
		}
		// 2stopbit
		// 2400Hz * 4 * 2
		set_tape_one_wave(tape_wave_nums << 2, 1);
	}
}

/// @param cnt [in] wave nums (half wave)
/// @param samp [in] 1:2400Hz
void CMT::set_tape_one_wave(int cnt, int samp)
{
	for(int j=0; j<cnt; j++) {
		for (int n=0; n<samp; n++) {
			set_tape_one_amp(tape_wave_onoff);
		}
		tape_wave_onoff = (1 - tape_wave_onoff);
	}
}

void CMT::set_tape_amp(char *c_data, int c_len)
{
	char *p = c_data;

	while(p != NULL && *p != '\0' && c_len > 0) {
		set_tape_one_amp(*p);
		p++;
		c_len--;
	}
}

/// wave amplitude
/// param[in] data : bit0:{1=plus 0=minus}
void CMT::set_tape_one_amp(char data)
{
	devide = (sample_rate + surplus) / tape_wav_freq;
	surplus = (sample_rate + surplus) - (devide * tape_wav_freq);

//	int a = tape_volume;
	int a = 16384;
	if ((data & 1) == 0) {
		a = -a;
	}
//	if (devide > 0 && tape_wave_buf_wpos < TAPEWAV_BUFFER_SIZE) {
	if (devide > 0 && !m_tape_wave.is_full()) {
		if (abs(a + a_prev + a_prev) > abs(a)) a_prev = 0;
//		tape_wave_buf[tape_wave_buf_wpos] = a + a_prev + a_prev;
//		tape_wave_buf_wpos++;
		m_tape_wave.add(a + a_prev + a_prev);
	}
//	for(int i=1; i<devide && tape_wave_buf_wpos < TAPEWAV_BUFFER_SIZE; i++) {
	for(int i=1; i<devide && !m_tape_wave.is_full(); i++) {
//		tape_wave_buf[tape_wave_buf_wpos] = a;
//		tape_wave_buf_wpos++;
		m_tape_wave.add(a);
	}
	a_prev = 0;
	if (surplus > 0) {
		a_prev = 16384 * surplus / tape_wav_freq;
//		a_prev = tape_volume * surplus / tape_wav_freq;
		if ((data & 1) == 0) {
			a_prev = -a_prev;
		}
	}
//	logging->out_debugf(_T("CMT:one_amp: wpos:%6d de:%2d sr:%4d d:%2d a:%6d a_pre:%6d"), tape_wave_buf_wpos, devide, surplus, data, a, a_prev);
}

void CMT::set_tape_direct(int16_t *w_data, int w_len)
{
//	for(int i=0; i<w_len && tape_wave_buf_wpos < TAPEWAV_BUFFER_SIZE; i++) {
//		tape_wave_buf[tape_wave_buf_wpos] = (int16_t)((int)w_data[i] * tape_volume * (pConfig->wav_correct ? 2 : 1) / 16384);
//		tape_wave_buf_wpos++;
//	}
	for(int i=0; i<w_len && !m_tape_wave.is_full(); i++) {
		int data = w_data[i];
		if (pConfig->wav_correct) data = ((data + data + data) >> 1);
		m_tape_wave.add(data);
	}
}

// ----------------------------------------------------------------------------
uint32_t CMT::get_cmt_mode()
{
	return (REG_REMOTE && play ? 0x01 : 0) | (REG_REMOTE && rec ? 0x02 : 0) | (play ? 0x04 : 0) | (rec ? 0x08 : 0) | (((data_total_pos / 800) % 12000) << 4);
}

// ----------------------------------------------------------------------------
void CMT::update_config()
{
	int cpu_power = (pConfig->sync_irq ? pConfig->cpu_power : 1);
	if (cpu_power >= 1) {
		sample_rate = sample_rate_base >> (cpu_power - 1);
	} else {
		sample_rate = sample_rate_base << (1 - cpu_power);
	}
	//
	wav->SetParseParamerers(pConfig->wav_half, pConfig->wav_reverse, pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0, pConfig->wav_correct_amp[0], pConfig->wav_correct_amp[1]);
}

// ----------------------------------------------------------------------------

void CMT::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(3);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));
	vm_state.flags = (REG_REMOTE ? 1 : 0) | (play ? 2 : 0) | (rec ? 4 : 0);
	vm_state.flags |= (send_ok ? 0x10 : 0) | (recv_ok ? 0x20 : 0);
	vm_state.flags |= (pConfig->NowRealModeDataRec() ? 0x80 : 0); // add version 2
	vm_state.baud_sel = REG_BAUD_SEL; // add version 3

	vm_state.register_id = Int32_LE(register_id[0]);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool CMT::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	REG_REMOTE = (vm_state.flags & 1) ? 0x80 : 0;
//	play   = (vm_state.flags & 2) ? true : false;
//	rec    = false;	// ignore record mode
	REG_BAUD_SEL = (vm_state.flags & 8) ? 1 : 0;
	send_ok = (vm_state.flags & 0x10) ? true : false;
	recv_ok = (vm_state.flags & 0x20) ? true : false;
	if (Uint16_LE(vm_state_i.version) >= 2) {
		pConfig->SetRealModeDataRec((vm_state.flags & 0x80) ? true : false);
	}
	if (Uint16_LE(vm_state_i.version) >= 3) {
		REG_BAUD_SEL = vm_state.baud_sel;
	}
	register_id[0] = Int32_LE(vm_state.register_id);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
bool CMT::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		write_signal(SIG_CMT_REMOTE, data, 0x80);
		return true;
	case 1:
		write_signal(SIG_CMT_BAUD_SEL, data, 0x0f);
		return true;
	}
	return false;
}

bool CMT::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

void CMT::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("REMOTE"), REG_REMOTE);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, _T("BAUD_SEL"), REG_BAUD_SEL);
}
#endif
