/** @file qt_rec_video.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2015.05.18 -

 @brief [ record video using quicktime ]
 */

#ifndef QT_RECORD_VIDEO_H
#define QT_RECORD_VIDEO_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_QTKIT)

#ifdef __OBJC__
#import <QTKit/QTKit.h>
#endif
#include "../../common.h"
#include "../../msgs.h"

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Record video using quicktime (obsolete)
*/
class QT_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;

	int rec_fps;

	int bpc;
	int bpp;
	int bpr;

	CSurface *rec_surface;

#ifdef __OBJC__
	QTMovie *movie;
	NSDictionary *exportType;
	NSDictionary *imageAttr;
	NSString *rec_file;
	NSString *tmp_file;
	CGColorSpaceRef color_space;
#else
	void *movie;
	void *exportType;
	void *imageAttr;
	void *rec_file;
	void *tmp_file;
	void *color_space;
#endif

public:
	QT_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~QT_REC_VIDEO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(const _TCHAR *path, size_t path_size, int fps, CSurface *surface, bool show_dialog);
	void Stop();
	bool Restart();
	bool Record();

	const _TCHAR **GetCodecList();
	const CMsg::Id *GetQualityList();
};

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_QTKIT */

#endif /* QT_RECORD_VIDEO_H */
