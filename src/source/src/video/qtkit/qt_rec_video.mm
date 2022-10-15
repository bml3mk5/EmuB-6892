/** @file qt_rec_video.mm

 Skelton for retropc emulator

 @author Sasaji
 @date   2015.05.18 -

 @brief [ record video using quicktime ]

 @note need MacOSX 10.5 and QuickTime 7.2.1 or later
 */

#import "qt_rec_video.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_QTKIT)

#import "../../depend.h"
#import "../rec_video.h"
#import "../../emu.h"
#import "../../gui/gui.h"
#import "../avkit/avk_rec_common.h"
#import "../../csurface.h"

QT_REC_VIDEO::QT_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;

	rec_fps = 0;

	bpc = 0;
	bpp = 0;
	bpr = 0;

	rec_surface = nil;

	movie = nil;
	rec_file = nil;
	tmp_file = nil;
	exportType = nil;
	imageAttr = nil;
	color_space = nil;
}

QT_REC_VIDEO::~QT_REC_VIDEO()
{
}

bool QT_REC_VIDEO::IsEnabled()
{
	return true;
}

/// @attention must call this method from emu thread
bool QT_REC_VIDEO::Start(const _TCHAR *path, size_t path_size, int fps, CSurface *surface, bool show_dialog)
{
	AllocPool(USE_REC_VIDEO_QTKIT);

	if (path != NULL) {
		// add extention
		CTchar cpath(path);
		rec_file = [NSString stringWithUTF8String:cpath.GetN()];
		rec_file = [rec_file stringByAppendingString:@".mov"];
		tmp_file = [NSString stringWithUTF8String:cpath.GetN()];
		tmp_file = [tmp_file stringByAppendingString:@".tmp"];
	}
	rec_fps = fps;

	if (rec_file == nil || rec_fps <= 0) {
		return false;
	}

	rec_surface = surface;

	if (rec_surface != NULL) {
		bpc = 8;
		bpp = rec_surface->BitsPerPixel(); // 32
		bpr = rec_surface->BytesPerLine();
	}
	exportType = [NSDictionary dictionaryWithObjectsAndKeys:
	  [NSNumber numberWithBool:YES], QTMovieFlatten,
	  [NSNumber numberWithBool:YES], QTMovieExport,
	  [NSNumber numberWithLong:kQTFileTypeMovie], QTMovieExportType,
	  nil];
	const NSString *codType[] = { @"avc1", @"jpeg", @"mp4v", @"mjpa", @"mjpb", @"tiff", @"raw ", nil };
	int codTypeNum = emu->get_parami(VM::ParamRecVideoCodec);
	if (codTypeNum < 0 || 6 < codTypeNum) codTypeNum = 0;
	const NSNumber *quoType[] = {[NSNumber numberWithLong:codecMaxQuality],
		[NSNumber numberWithLong:codecHighQuality],
		[NSNumber numberWithLong:codecNormalQuality],
		[NSNumber numberWithLong:codecLowQuality],
		[NSNumber numberWithLong:codecMinQuality],
		nil };
	int quoTypeNum = emu->get_parami(VM::ParamRecVideoQuality);
	if (quoTypeNum < 0 || 4 < quoTypeNum) quoTypeNum = 0;
	imageAttr = [NSDictionary dictionaryWithObjectsAndKeys:
		 codType[codTypeNum], QTAddImageCodecType,
		 quoType[quoTypeNum], QTAddImageCodecQuality,
		 [NSNumber numberWithLong:3600], QTTrackTimeScaleAttribute,
		 nil];

	if (color_space == nil) {
		color_space = CGColorSpaceCreateDeviceRGB();
//		const CGFloat whitePoint[3]={ 1.0, 1.0, 1.0 };
//		const CGFloat blackPoint[3]={ 0.0, 0.0, 0.0 };
//		const CGFloat gamma[3]={ 1.0, 1.2, 1.2 };
//		const CGFloat matrix[9]={ 0,0,1,  0,1,0,  1,0,0 };
//		color_space = CGColorSpaceCreateCalibratedRGB(whitePoint,blackPoint,gamma,matrix);
	}

	// create movie stream for store image frames
	NSError *err = nil;
//	movie = [[QTMovie alloc] initToWritableData:[NSMutableData data] error:&err];
//	QTDataReference *dataref = [QTDataReference dataReferenceWithReferenceToFile:tmp_file];
//	movie = [[QTMovie alloc] initToWritableDataReference:dataref error:&err];
	movie = [[QTMovie alloc] initToWritableFile:tmp_file error:&err];
	if (err) {
		logging->out_logf(LOG_ERROR, "QTMovie initToWritableFile Failed. %s", [[err description] UTF8String]);
		return false;
	}
	[movie setCurrentTime:QTMakeTime(0, rec_fps)];

	return true;
}

/// @attention must call this method from emu thread
void QT_REC_VIDEO::Stop()
{
	NSError *err = nil;
	emu->out_msg_x(CMsg::Now_saving_video_file_,true,-1);
	emu->get_gui()->NeedUpdateScreen();
	SDL_Delay(100);

	BOOL rc = [movie writeToFile:rec_file withAttributes:exportType error:&err];
	if (!rc) {
		logging->out_logf(LOG_ERROR, "Stop: QTMovie writeToFile Failed. %s", [[err description] UTF8String]);
	}
	[movie invalidate];
	movie = nil;
	unlink([tmp_file UTF8String]);

	emu->out_msg_x(CMsg::Now_saving_video_file_,false,-1);
	if (rc) emu->out_msg_x(CMsg::Video_file_was_saved);

	ReleasePool(USE_REC_VIDEO_QTKIT);
}

bool QT_REC_VIDEO::Restart()
{
	vid->Stop();
	return vid->Start(RECORD_VIDEO_TYPE_QTKIT, -1, vid->GetSrcRect(), NULL, false);
}

/// @attention must call this method from emu thread
bool QT_REC_VIDEO::Record()
{
	NSAutoreleasePool *pool_rec = [[NSAutoreleasePool alloc] init];

	rec_surface->Lock();
#if 1
	void *buffer = rec_surface->GetBuffer();
	uint32_t buffer_size = rec_surface->GetBufferSize();

	CGBitmapInfo bi = kCGBitmapByteOrderDefault;
	CGDataProviderRef pv = CGDataProviderCreateWithData(NULL, (const void *)buffer, buffer_size, NULL);

	CGImageRef imageref = CGImageCreate(rec_surface->Width(), rec_surface->Height(), bpc, bpp, bpr, color_space, bi, pv, NULL, false, kCGRenderingIntentDefault);

	NSImage *image = [[NSImage alloc] initWithCGImage:imageref size:NSZeroSize];
#else
	NSImage *image = [[NSImage alloc] initWithContentsOfFile:@"igdemo.jpg"];
#endif
	[movie addImage:image forDuration:QTMakeTime(1, rec_fps) withAttributes:imageAttr];

	[movie setCurrentTime:[movie duration]];
//	[movie setCurrentTime:QTMakeTime(frame_count, rec_fps)];

	rec_surface->Unlock();

	[image release];
	CGImageRelease(imageref);
	CGDataProviderRelease(pv);

	[pool_rec release];
	return true;
}

const _TCHAR **QT_REC_VIDEO::GetCodecList()
{
	static const _TCHAR *list[] = {_T("H.264"),_T("JPEG"),_T("MPEG-4"),_T("Motion JPEG A"),_T("Motion JPEG B"),_T("TIFF"),_T("Raw"),NULL};
	return list;
}

const CMsg::Id *QT_REC_VIDEO::GetQualityList()
{
	static const CMsg::Id list[] = {CMsg::Max,CMsg::High,CMsg::Normal,CMsg::Low,CMsg::Min,CMsg::End};
	return list;
}

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_QTKIT */
