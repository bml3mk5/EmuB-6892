/** @file mmf_rec_audio.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ record audio using microsoft media foundation ]

	@note This component can use on Windows7 or later.
*/

#ifndef _MMF_RECORD_AUDIO_H_
#define _MMF_RECORD_AUDIO_H_

#if defined(_WIN32)
#include <windows.h>
#endif
#include "../../rec_video_defs.h"
#include "../../common.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_MMF)

#include "mmf_loadlib.h"

class EMU;
class REC_AUDIO;

/**
	@brief Record audio using microsoft media foundation (Windows 7 or lator)
*/
class MMF_REC_AUDIO
{
private:
	EMU *emu;
	REC_AUDIO *audio;
	bool enable;
	int rec_rate;
	const _TCHAR *rec_path;

	int store_sample_pos;

	DWORD streamIndex;
	LONGLONG sample_time;
	LONGLONG frame_duration;
	UINT32 bytes_per_sample;
	int sample_buffer_size;
	int sample_is_multiple;
	int write_error_count;

	IMFSinkWriter *sinkWriter;
	IMFMediaBuffer *mediaBuffers[2];

	void Release();
	void RemoveFile();

	HRESULT WriteSample();

public:
	MMF_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio);
	~MMF_REC_AUDIO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(_TCHAR *path, size_t path_size, int sample_rate);
	void Stop();
	bool Restart();
//	bool Record(uint8_t *buffer, int samples);
//	bool Record(int16_t *buffer, int samples);
	bool Record(int32_t *buffer, int samples);

	const _TCHAR **GetCodecList();
};

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_MMF */

#endif /* _MMF_RECORD_AUDIO_H_ */
