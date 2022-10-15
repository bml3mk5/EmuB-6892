/** @file mmf_rec_video.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ record video using microsoft media foundation ]

	@note This component can use on Windows7 or later.
*/

#ifndef _MMF_RECORD_VIDEO_H_
#define _MMF_RECORD_VIDEO_H_

#if defined(_WIN32)
#include <windows.h>
#endif
#include "../../rec_video_defs.h"
#include "../../common.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_MMF)

#include "mmf_loadlib.h"
#include "../../msgs.h"

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Record video using microsoft media foundation (Windows 7 or lator)
*/
class MMF_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;
	int rec_fps;
	VmRectWH rec_rect;
	const _TCHAR *rec_path;

	CSurface *rec_surface;

	DWORD streamIndex;
	LONGLONG sample_time;
	LONGLONG frame_duration;
	int stride;
	int write_error_count;

	IMFSinkWriter *sinkWriter;

	void Release();
	void RemoveFile();

	HRESULT WriteSample(IMFMediaBuffer *pBuffer);

public:
	MMF_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~MMF_REC_VIDEO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(_TCHAR *path, size_t path_size, int fps, const VmRectWH *srcrect, CSurface *srcsurface, bool show_dialog);
	void Stop();
	bool Restart();
	bool Record();

	const _TCHAR **GetCodecList();
	const CMsg::Id *GetQualityList();
};

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_MMF */

#endif /* _MMF_RECORD_VIDEO_H_ */
