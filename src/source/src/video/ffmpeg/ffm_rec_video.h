/** @file ffm_rec_video.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ record video using ffmpeg ]

	@note
	If use this, get ffmpeg library from http://www.ffmpeg.org/.

	To use on Windows:
	Get FFmpeg "shared" package from http://ffmpeg.zeranoe.com/builds/
	and put dll files on this application folder.
*/

#ifndef FFM_RECORD_VIDEO_H
#define FFM_RECORD_VIDEO_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_FFMPEG)

#include "../../common.h"
#include "ffm_loadlib.h"
#include "../../msgs.h"

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Record video using ffmpeg
*/
class FFM_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;
	int rec_fps;
	VmRectWH rec_rect;
	const _TCHAR *rec_path;

	CSurface *rec_surface;

	AVIOContext *avio;
	AVFormatContext *fmtcont;
	AVCodecContext *codcont;
	AVFrame *frame;
	AVFrame *srcframe;
	SwsContext *sws_cont;
	AVPacket *packet;

	int write_error_count;

	void Release();
	void RemoveFile();

	bool Encoding(AVFrame *frame, AVPacket *packet);

public:
	FFM_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~FFM_REC_VIDEO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(_TCHAR *path, size_t path_size, int fps, const VmRectWH *srcrect, CSurface *srcsurface, bool show_dialog);
	void Stop();
	bool Restart();
	bool Record();

	const _TCHAR **GetCodecList();
	const CMsg::Id *GetQualityList();

};

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_FFMPEG */

#endif /* FFM_RECORD_VIDEO_H */
