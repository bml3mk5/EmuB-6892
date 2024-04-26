/** @file ffm_rec_base.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.12.18 -

	@brief [ record video using ffmpeg ]

	@note
	If use this, get ffmpeg library from http://www.ffmpeg.org/.
*/

#ifndef FFM_RECORD_BASE_H
#define FFM_RECORD_BASE_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_FFMPEG)

#include "../../common.h"
#include "ffm_loadlib.h"
#include "../../cchar.h"
#include "../../msgs.h"

class EMU;
class REC_VIDEO;
class CSurface;

/// supported codec table of ffmpeg
struct st_codec_ids {
	AVCodecID id;
	const char *name;
	const _TCHAR *ext;
};

/**
	@brief Record video/audio using ffmpeg
*/
class FFM_REC_BASE
{
protected:
	EMU *emu;

	CTchar crec_path;

	AVIOContext *avio;
	AVFormatContext *fmtcont;
	AVOutputFormat *oformat;
	AVCodecContext *codcont;
	AVFrame *frame;
	AVFrame *srcframe;
	AVPacket *packet;

	int write_error_count;

	void Release();
	void RemoveFile();

	enum enOutputType {
		TYPE_VIDEO = 1,
		TYPE_AUDIO = 2,
	};
	int AddStream(enOutputType type, AVCodecID codec_id, const char *codec_name, AVCodec *&codec, AVStream *&stream);

public:
	FFM_REC_BASE(EMU *new_emu);
	virtual ~FFM_REC_BASE();

	virtual bool IsEnabled() = 0;
};

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_FFMPEG */

#endif /* FFM_RECORD_BASE_H */
