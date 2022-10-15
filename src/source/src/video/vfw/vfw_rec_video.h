/** @file vfw_rec_video.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ record video using video for windows ]
*/

#ifndef VFW_RECORD_VIDEO_H
#define VFW_RECORD_VIDEO_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_VFW)

#include "../../common.h"
#include <windows.h>
#include <vfw.h>

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Record video using video for windows
*/
class VFW_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;
	int rec_fps;
	VmRectWH rec_rect;
	const _TCHAR *rec_path;

	CSurface *rec_surface;

	// vfw
	PAVIFILE pAVIFile;
	PAVISTREAM pAVIStream;
	PAVISTREAM pAVICompressed;
	AVICOMPRESSOPTIONS opts;
	DWORD dwAVIFileSize;
	LONG lAVIFrames;

	BITMAPINFO bmi;

	void Release();
	void RemoveFile();

public:
	VFW_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~VFW_REC_VIDEO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(_TCHAR *path, size_t path_size, int fps, const VmRectWH *srcrect, CSurface *srcsurface, bool show_dialog);
	void Stop();
	bool Restart();
	bool Record();

	const _TCHAR **GetCodecList();
	const _TCHAR **GetQualityList();

};

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_VFW */

#endif /* VFW_RECORD_VIDEO_H */
