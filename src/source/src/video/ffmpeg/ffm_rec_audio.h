/** @file ffm_rec_audio.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio using ffmpeg ]

	@note
	If use this, get ffmpeg library from http://www.ffmpeg.org/.

	To use on Windows:
	Get FFmpeg "shared" package from http://ffmpeg.zeranoe.com/builds/
	and put dll files on this application folder.
*/

#ifndef FFM_RECORD_AUDIO_H
#define FFM_RECORD_AUDIO_H

#if defined(_WIN32)
#include <windows.h>
#endif
#include "../../rec_video_defs.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_FFMPEG)

#include "../../common.h"
#include "ffm_loadlib.h"
#include "ffm_rec_base.h"
#include "libavutil/channel_layout.h"

class EMU;
class REC_AUDIO;

/**
	@brief Record audio using ffmpeg
*/
class FFM_REC_AUDIO : public FFM_REC_BASE
{
private:
	REC_AUDIO *audio;
	int rec_rate;

	union {
		int16_t *s;
		float *f;
	} store_samples[2];
	int store_sample_pos;

	void Release();

	int SelectSampleRate(AVCodec *codec, int default_rate);
	enum AVSampleFormat SelectSampleFmt(AVCodec *codec);

	bool Encoding(AVFrame *frame, AVPacket *packet);

public:
	FFM_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio);
	~FFM_REC_AUDIO();

	bool IsEnabled();

	bool Start(_TCHAR *path, size_t path_size, int sample_rate);
	void Stop();
	bool Restart();
	bool Record(int32_t *buffer, int samples);

	const _TCHAR **GetCodecList();
};

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_FFMPEG */

#endif /* FFM_RECORD_AUDIO_H */
