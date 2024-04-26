/** @file avk_rec_audio.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.11.03 -

 @brief [ record audio using AVFoundation ]
 */

#ifndef AVK_RECORD_AUDIO_H
#define AVK_RECORD_AUDIO_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_AVKIT)

#ifdef __OBJC__
#import <AVFoundation/AVFoundation.h>
#endif
#include "../../common.h"

class EMU;
class REC_AUDIO;

/**
	@brief Record audio using AVFoundation on MacOSX
*/
class AVK_REC_AUDIO
{
private:
	EMU *emu;
	REC_AUDIO *audio;

	int rec_rate;
	int frame_count;
	size_t store_sample_pos;
	bool use_float;
	int sample_idx;

	int write_error_count;

#ifdef __OBJC__
	AVAssetWriter *asset;
	AVAssetWriterInput *input;
//	CMBlockBufferRef store_buffer;
	CMSampleBufferRef sample_buffer[2];
	NSString *rec_file;

//	AVAsset *oya;
//	AVAssetReader *assetReader;
//	AVAssetReaderOutput *output;
#else
	void *asset;
	void *input;
//	void *store_buffer;
	void *sample_buffer[2];
	void *rec_file;

//	void *oya;
//	void *assetReader;
//	void *output;
#endif

public:
	AVK_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio);
	~AVK_REC_AUDIO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(const _TCHAR *path, size_t path_size, int sample_rate);
	void Stop();
	bool Restart();
	//	bool Record(uint8_t *buffer, int samples);
	//	bool Record(int16_t *buffer, int samples);
	bool Record(int32_t *buffer, int samples);

	const _TCHAR **GetCodecList();
};

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_AVKIT */

#endif /* AVK_RECORD_AUDIO_H */
