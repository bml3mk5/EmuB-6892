/** @file sound.cpp

	HITACHI BASIC MASTER LEVEL3 Mark5 Emulator 'EmuB-6892'
	Skelton for retropc emulator

	@par Origin
	pcm1bit.cpp
	@author Sasaji
	@date   2012.01.07 -

	@brief [ sound ]
*/

#define _USE_MATH_DEFINES
#include <math.h>
#include "sound.h"
//#include "../../emu.h"
#include "../vm.h"
#include "../../fileio.h"
#include "../../config.h"

#ifdef SOUND_HIGH_QUALITY
#if USE_SOUND_FILTER == 3
int	RRX11 = 12;
int RRX12 = 12;
int RRY11 = 976;
#define VOLM 1000
#else
#define	RRA 60
#endif
#endif

void SOUND::initialize()
{
	signal = false;
	on = false;
	mute = false;
	prev_on = false;

#ifdef SOUND_HIGH_QUALITY
	sample_rate  = 48000;
//	sample_clock = CPU_CLOCKS / sample_rate;
	wclk = (CPU_CLOCKS * FILTER_SAMPLES / SAMPLE_RATE_MAX);
	widx[0]=0;
	dclk[0]=0;
	wsample = SAMPLE_RATE_MAX;
	wresidue = 0;
	dclocks = CPU_CLOCKS;
	dresidue = 0;
	prev_event_power = 1;
	update_config();
	cur_clock_s = cur_clock_c = 0;
	last_registred_clock = 0;
#if USE_SOUND_FILTER == 3
	memset(out_vals, 0, sizeof(out_vals));
	memset(prev_vals, 0, sizeof(prev_vals));
#endif
#endif
	prev_vol = 0;

	update = 0;

//	register_frame_event(this);
}

void SOUND::reset()
{
	signal = false;
	on = false;
	mute = false;
	prev_on = false;

#ifdef SOUND_HIGH_QUALITY
	samples.clear();
	samples.add(false, false, 0);
#endif
	update = 0;

}

void SOUND::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SOUND_SIGNAL) {
		bool next = ((data & mask) != 0);
		if (signal != next || on != prev_on) {
#ifdef SOUND_HIGH_QUALITY
			last_registred_clock = vm->get_current_clock();
			samples.add(next, (on && !mute), last_registred_clock);
#endif
			// mute if signal is not changed
//			update = 60;
			update = 10;
			signal = next;
		}
	}
	else if(id == SIG_SOUND_ON) {
		prev_on = on;
		on = ((data & mask) != 0);
	}
	else if(id == SIG_SOUND_MUTE) {
		mute = ((data & mask) != 0);
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void SOUND::event_frame()
{
}

void SOUND::mix(int32_t* buffer, int cnt)
{
#ifdef SOUND_HIGH_QUALITY
	if (cnt <= 0) return;
	if (cnt > MAX_OUT_COUNTS) cnt = MAX_OUT_COUNTS;

	// set sample window range
	windex = 0;
	dindex = 0;
	for(int i=1; i<=cnt; i++) {
		widx[i] = (wsample + wresidue) / (sample_rate * 2);
		wresidue = (wsample + wresidue) % (sample_rate * 2);
		widx[i] += windex;
		windex = widx[i];
		if (widx[i] >= FILTER_SAMPLES) {
			widx[i] = FILTER_SAMPLES - 1;
		}
		dclk[i] = (dclocks + dresidue) / (sample_rate * 2);
		dresidue = (dclocks + dresidue) % (sample_rate * 2);
		dclk[i] += dindex;
		dindex = dclk[i];
	}

	//
	cur_clock_s += dclk[cnt];
	cur_clock_c = vm->get_current_clock() - wclk;
//	logging->out_debugf(_T("SOUND: s:%lld c:%lld cnt:%d del_clock:%lld"),cur_clock_s,cur_clock_c,cnt,dclk[cnt]);
	if (cur_clock_s + 400 < cur_clock_c) cur_clock_s += 400;
	if (cur_clock_s > cur_clock_c + 4) cur_clock_s = cur_clock_c;
//	logging->out_debugf(_T("---->  s:%lld c:%lld"),cur_clock_s,cur_clock_c);

	// get sample data
	get_sample_data(wclk, cur_clock_s);

	// mix sample data
#if USE_SOUND_FILTER == 3
	for (int n = 0; n < 1; n++) {
		out_vals[n + widx[cnt]] = prev_vals[n];
	}
	for (int n = widx[cnt] - 1; n >= 0; n--) {
		out_vals[n] = (in_vals[n]*RRX11 + in_vals[n+1]*RRX12 + out_vals[n+1]*RRY11) / 1000;
	}
//	memcpy(prev_vals, out_vals, sizeof(prev_vals));
	for (int n = 0; n < 1; n++) {
		prev_vals[n] = out_vals[n];
	}

	for(int i = cnt - 1; i >= 0; i--) {
		int cur_vol = 0;
		for (int n = widx[i+1] - 1; n >= widx[i]; n--) {
//			cur_vol += (int)((int64_t)out_vals[n] * max_vol * sound_volume / VOLM / 100);
			cur_vol += out_vals[n];
		}
		if (widx[i+1] > widx[i]) cur_vol /= (widx[i+1]-widx[i]);
		// out to sound buffer
		*buffer++ += cur_vol; // L
		*buffer++ += cur_vol; // R
//		prev_vol = cur_vol;
	}

#else
	for(int i = cnt - 1; i >= 0; i--) {
		int cur_vol = 0;
		for(int n = widx[i+1] - 1; n >= widx[i]; n--) {
			cur_vol += in_vals[n] * max_vol / 800;
		}
		if (widx[i+1] > widx[i]) cur_vol /= (widx[i+1]-widx[i]);
		cur_vol = cur_vol * sound_volume / 100;
//		int p = widx[i];
//		cur_vol += in_vals[p] * max_vol / 800;
//		cur_vol = (cur_vol * RRA + prev_vol * (100 - RRA)) / 100;
		// out to sound buffer
		*buffer++ += cur_vol; // L
		*buffer++ += cur_vol; // R
		prev_vol = cur_vol;
	}
#endif
#else
	if(on && !mute && update) {
		int cur_vol = signal ? max_vol : -max_vol;
		for(int i = 0; i < cnt; i++) {
			cur_vol = cur_vol * sound_volume / 100;
			cur_vol = (cur_vol * 7 + prev_vol * 3) / 10;
			*buffer++ += cur_vol; // L
			*buffer++ += cur_vol; // R
			prev_vol = cur_vol;
		}
	}
#endif

	// auto off
#ifndef SOUND_HIGH_QUALITY
	if(update && --update == 0) {
		signal = false;
		prev_on = on;
		on = false;
//		samples.add(signal, (on && !mute), vm->get_current_clock());
	}
#endif
}

void SOUND::set_volume(int decibel, bool vol_mute)
{
	mute = vol_mute;
	sound_volume = int(16384.0 * pow(10.0, decibel / 40.0));
	if (sound_volume < 1) sound_volume = 1;
}

void SOUND::initialize_sound(int rate, int decibel)
{
#ifdef SOUND_HIGH_QUALITY
	sample_rate = rate;
//	sample_clock = CPU_CLOCKS / rate;
	set_widx();
#endif
	set_volume(decibel, false);
}

void SOUND::set_widx()
{
#ifdef SOUND_HIGH_QUALITY
	wsample = wsample / prev_event_power * (1 << event_power);
	dclocks = dclocks / prev_event_power * (1 << event_power);
	wresidue = wresidue / prev_event_power * (1 << event_power);
	dresidue = dresidue / prev_event_power * (1 << event_power);

	prev_event_power = (1 << event_power);
#endif
}

void SOUND::get_sample_data(uint64_t window_clock, uint64_t end_clock)
{
#ifdef SOUND_HIGH_QUALITY
	int start_index = -1;
	int cur_window = 0;
	int prev_window = -1;
	int s = 0;
	int n = 0;
	int start_gain = 0;
	uint64_t start_clock = end_clock - window_clock;

//	memset(in_vals, 0, sizeof(in_vals));
	for(s = 0; s < FILTER_SAMPLES; s++) {
		in_vals[s] = 0;
	}

	// get sample data from turn on/off point
	for(s = samples.count - 1; s >= 0; s--) {

		uint64_t clock = samples.get_clock(s);
		if(clock <= end_clock) {
			if (end_clock > clock + 999999) {
				cur_window = 999999;
			} else {
				cur_window = (int)((end_clock - clock) * FILTER_SAMPLES / window_clock);
			}
			if (cur_window > prev_window + FILTER_SAMPLE_NUM) {
				prev_window = cur_window - FILTER_SAMPLE_NUM - 1;
			}
			start_gain = FILTER_SAMPLE_NUM;
			if (cur_window >= FILTER_SAMPLES) {
				start_gain = FILTER_SAMPLE_NUM - cur_window + FILTER_SAMPLES;
				if (start_gain < 0) start_gain = 0;
				cur_window = FILTER_SAMPLES - 1;
			}

			if(samples.get_out(s)) {
				if(samples.get_signal(s)) {
					for(n = cur_window; n > prev_window; n--) {
						in_vals[n] = sound_volume * start_gain / FILTER_SAMPLE_NUM;
						if (start_gain > 0) {
							start_gain--;
						}
					}
				} else {
					for(n = cur_window; n > prev_window; n--) {
						in_vals[n] = -sound_volume * start_gain / FILTER_SAMPLE_NUM;
						if (start_gain > 0) {
							start_gain--;
						}
					}
				}
			}
			prev_window = cur_window;
			start_index = s;
		}

		if (clock < start_clock) {
			break;
		}

	}

	// shift used points
	samples.shift(start_index);
#endif
}

// ----------------------------------------------------------------------------

void SOUND::update_config()
{
	if (config.sync_irq) {
		event_power = config.cpu_power;
	} else {
		event_power = 1;
	}
	set_widx();
}

// ----------------------------------------------------------------------------

void SOUND::save_state(FILEIO *fp)
{
}

bool SOUND::load_state(FILEIO *fp)
{
#ifdef SOUND_HIGH_QUALITY
	cur_clock_c = vm->get_current_clock() - (CPU_CLOCKS * FILTER_SAMPLES / SAMPLE_RATE_MAX);
	cur_clock_s = cur_clock_c;
#endif

	return true;
}
