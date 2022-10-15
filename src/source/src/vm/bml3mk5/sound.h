/** @file sound.h

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@par Origin
	pcm1bit.h
	@author Sasaji
	@date   2012.01.07 -

	@brief [ sound ]
*/

#ifndef SOUND_H
#define SOUND_H

#include "../vm_defs.h"
#include "../device.h"

#define SOUND_HIGH_QUALITY

#ifdef SOUND_HIGH_QUALITY
#define USE_SOUND_FILTER	3

#define MAX_SAMPLES		1024

#define MAX_OUT_COUNTS	20

#ifdef USE_SOUND_FILTER
#if USE_SOUND_FILTER == 3
#define SAMPLE_RATE_MAX CPU_CLOCKS
#define FILTER_SAMPLES	128
#define FILTER_SAMPLE_NUM	9600
#else
#define SAMPLE_RATE_MAX 96000
#define FILTER_SAMPLES	32
#define FILTER_SAMPLE_NUM	3200
#define PREV_VOLS_MAX	16
#endif
#endif

class EMU;

/**
	@brief record turn on/off point of MUSIC_SEL register

	turn on/offする位置を記録するクラス
*/
class SOUND_SAMPLE_POINT
{
private:
	bool   sigs[MAX_SAMPLES];
	bool   outs[MAX_SAMPLES];
	uint64_t clks[MAX_SAMPLES];

	bool   stored;
	bool   cur_signal;
	bool   cur_out;
public:
	int    count;
	void add(bool signal, bool out, uint64_t clk) {
		if (count >= MAX_SAMPLES) return;
		if (stored) {
			if (signal == cur_signal && out == cur_out) return;
		}

		sigs[count] = signal;
		outs[count] = out;
		clks[count] = clk;
		count++;

		cur_signal = signal;
		cur_out = out;
		stored = true;
	}
	void clear() {
		for(int i=0; i<MAX_SAMPLES; i++) {
			sigs[i] = false;
			outs[i] = false;
			clks[i] = 0;
		}
		count = 0;
		stored = false;
	}
	bool get_signal(int pos) {
		return sigs[pos];
	}
	bool get_out(int pos) {
		return outs[pos];
	}
	uint64_t get_clock(int pos) {
		return clks[pos];
	}
	void shift(int cnt) {
		if (cnt > 0) {
			for (int s = 0; s < (count - cnt); s++) {
				sigs[s] = sigs[s + cnt];
				outs[s] = outs[s + cnt];
				clks[s] = clks[s + cnt];
			}
			if (count > cnt) {
				sigs[count-cnt] = false;
				outs[count-cnt] = false;
				clks[count-cnt] = 0;

				count -= cnt;
			} else {
				sigs[0] = false;
				outs[0] = false;
				clks[0] = 0;

				count = 0;
			}
		}
	}
};
#endif	/* SOUND_HIGH_QUALITY */

class FILEIO;

/**
	@brief sound - MUSIC_SEL register
*/
class SOUND : public DEVICE
{
public:
	/// @brief signals on SOUND
	enum SIG_SOUND_IDS {
		SIG_SOUND_SIGNAL	= 0,
		SIG_SOUND_ON		= 1,
		SIG_SOUND_MUTE		= 2
	};

private:
	bool signal, on, mute;
	bool prev_on;

#ifdef SOUND_HIGH_QUALITY
	SOUND_SAMPLE_POINT samples;

	int sample_rate;
//	int sample_clock;
	uint64_t cur_clock_s, cur_clock_c;
	uint64_t last_registred_clock;
	int event_power;
	int in_vals[FILTER_SAMPLES];
	int widx[MAX_OUT_COUNTS+1];
	int dclk[MAX_OUT_COUNTS+1];
	int wclk;
	int wsample;
	int windex;
	int wresidue;
	int dclocks;
	int dindex;
	int dresidue;
	int prev_event_power;
#if USE_SOUND_FILTER == 3
	int out_vals[FILTER_SAMPLES + 10];
	int prev_vals[FILTER_SAMPLES + 10];
#endif
#endif
	int sound_volume;
	int update;
	int prev_vol;

	void get_sample_data(uint64_t window_clock, uint64_t end_clock);
	void set_widx();

public:
	SOUND(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("SOUND");
	}
	~SOUND() {}

	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel, bool vol_mute);

	void update_config();

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

	// unique function
	void initialize_sound(int rate, int decibel);
};

#endif /* SOUND_H */
