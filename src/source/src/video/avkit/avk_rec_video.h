/** @file avk_rec_video.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.11.03 -

 @brief [ record video using AVFoundation ]
 */

#ifndef AVK_RECORD_VIDEO_H
#define AVK_RECORD_VIDEO_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_AVKIT)

#ifdef __OBJC__
#import <AVFoundation/AVFoundation.h>
#endif
#include "../../common.h"
#include "../../msgs.h"

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Record video using AVFoundation on MacOSX
*/
class AVK_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;

	int rec_fps;
	int frame_count;

	int bpc;
	int bpp;
	int bpr;

	CSurface *rec_surface;

#ifdef __OBJC__
	AVAssetWriter *asset;
	AVAssetWriterInput *input;
	AVAssetWriterInputPixelBufferAdaptor *input_adaptor;
	NSString *rec_file;
	CGColorSpaceRef color_space;
#ifdef USE_AVK_CREATE_PIXEL_BUFFER
	CVPixelBufferRef pixel_buffer;
#endif
#else
	void *asset;
	void *input;
	void *input_adaptor;
	void *rec_file;
	void *color_space;
#ifdef USE_AVK_CREATE_PIXEL_BUFFER
	void *pixel_buffer;
#endif
#endif

public:
	AVK_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~AVK_REC_VIDEO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(const _TCHAR *path, size_t path_size, int fps, CSurface *surface, bool show_dialog);
	void Stop();
	bool Restart();
	bool Record();

	const _TCHAR **GetCodecList();
	const CMsg::Id *GetQualityList();
};

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_AVKIT */

#endif /* AVK_RECORD_VIDEO_H */
