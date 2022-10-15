/** @file avk_rec_video.mm

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.11.03 -

 @brief [ record video using AVFoundation ]

 @note need MacOSX 10.7 or later
 */

#import "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_AVKIT)

//#import "CoreVideo/CVPixelBuffer.h"
#import "../../depend.h"
#import "avk_rec_video.h"
#import "../rec_video.h"
#import "../../emu.h"
#import "../../gui/gui.h"
#import "avk_rec_common.h"
#import "../../csurface.h"

AVK_REC_VIDEO::AVK_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;

	rec_fps = 0;
	frame_count = 0;

	bpc = 0;
	bpp = 0;
	bpr = 0;

	rec_surface = nil;

	asset = nil;
	input_adaptor = nil;
	rec_file = nil;
	color_space = nil;
#ifdef USE_AVK_CREATE_PIXEL_BUFFER
	pixel_buffer = nil;
#endif
}

AVK_REC_VIDEO::~AVK_REC_VIDEO()
{
}

bool AVK_REC_VIDEO::IsEnabled()
{
	return true;
}

enum codec_ids {
	CODEC_M4V = 0,
	CODEC_MP4,
	CODEC_QTV,
	CODEC_UNKNOWN
};

typedef struct {
	enum codec_ids   num;
	NSString * const fil;
	NSString * const cod;
	NSString * const ext;
} codType_t;

static const codType_t codType[] = {
	{ CODEC_M4V, AVFileTypeAppleM4V,       AVVideoCodecH264, @".m4v" },
	{ CODEC_MP4, AVFileTypeMPEG4,          AVVideoCodecH264, @".mp4" },
	{ CODEC_QTV, AVFileTypeQuickTimeMovie, AVVideoCodecH264, @".mov" },
};

const _TCHAR **AVK_REC_VIDEO::GetCodecList()
{
	static const _TCHAR *list[] = {
		_T("M4V (H.264)"),
		_T("MPEG4 (H.264)"),
		_T("QuickTime (H.264)"),
		NULL };
	return list;
}

static const int quoType[] = {
	50000000,
	10000000,
	1000000,
	500000,
	100000
};

const CMsg::Id *AVK_REC_VIDEO::GetQualityList()
{
	static const CMsg::Id list[] = {
		CMsg::Max_50Mbps,
		CMsg::High_10Mbps,
		CMsg::Normal_1Mbps,
		CMsg::Low_500Kbps,
		CMsg::Min_100Kbps,
		CMsg::End };
	return list;
}

/// @attention must call this method from emu thread
bool AVK_REC_VIDEO::Start(const _TCHAR *path, size_t path_size, int fps, CSurface *surface, bool show_dialog)
{
	AllocPool(USE_REC_VIDEO_AVKIT);

	int codTypeNum = emu->get_parami(VM::ParamRecVideoCodec);
	if (codTypeNum < 0 || CODEC_UNKNOWN <= codTypeNum) codTypeNum = 0;

	int quoTypeNum = emu->get_parami(VM::ParamRecVideoQuality);
	if (quoTypeNum < 0 || 4 < quoTypeNum) quoTypeNum = 0;

	if (path != NULL) {
		// add extention
		CTchar cpath(path);
		rec_file = [NSString stringWithUTF8String:cpath.GetN()];
		rec_file = [rec_file stringByAppendingString:codType[codTypeNum].ext];
	}
	rec_fps = fps;
	frame_count = 0;

	if (rec_file == nil || rec_fps <= 0) {
		return false;
	}

	rec_surface = surface;

	if (rec_surface != NULL) {
		bpc = 8;
		bpp = rec_surface->BitsPerPixel(); // 32
		bpr = rec_surface->BytesPerLine();
	}

	if (color_space == nil) {
		color_space = CGColorSpaceCreateDeviceRGB();
	}

	// create movie stream for store image frames
	NSError *err;
	NSURL *rec_url = [NSURL fileURLWithPath:rec_file];
	asset = [AVAssetWriter assetWriterWithURL:rec_url fileType:codType[codTypeNum].fil error:&err];
	if (!asset) {
		logging->out_log(LOG_ERROR, "AVAssetWriter init Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}

	// input type
	// AVVideoCompressionPropertiesKey
	NSDictionary *compattr = @{
		AVVideoAverageBitRateKey: [NSNumber numberWithInt:quoType[quoTypeNum]]
    };

	NSDictionary *settings = @{
		AVVideoCodecKey  : codType[codTypeNum].cod,
		AVVideoWidthKey  : [NSNumber numberWithInteger: rec_surface->Width()],
		AVVideoHeightKey : [NSNumber numberWithInteger: rec_surface->Height()],
		AVVideoCompressionPropertiesKey : compattr
	};

	input = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:settings];
	if (![asset canAddInput:input]) {
		err = [asset error];
		logging->out_log(LOG_ERROR, "AVAssetWriter addInput Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}
	[asset addInput:input];

	// pixel buffer
	NSDictionary *attr = @{
		(NSString *)kCVPixelBufferCGImageCompatibilityKey : [NSNumber numberWithBool:YES],
		(NSString *)kCVPixelBufferCGBitmapContextCompatibilityKey : [NSNumber numberWithBool:YES],
		(NSString *)kCVPixelBufferPixelFormatTypeKey : [NSNumber numberWithInt:kCVPixelFormatType_32ARGB],
		(NSString *)kCVPixelBufferWidthKey : [NSNumber numberWithInt: rec_surface->Width()],
		(NSString *)kCVPixelBufferHeightKey : [NSNumber numberWithInt: rec_surface->Height()]
	};

	input_adaptor = [AVAssetWriterInputPixelBufferAdaptor
	 assetWriterInputPixelBufferAdaptorWithAssetWriterInput:input
	 sourcePixelBufferAttributes:attr];

#ifdef USE_AVK_CREATE_PIXEL_BUFFER
	// create pixel buffer
	if (CVPixelBufferCreate(kCFAllocatorDefault, rec_surface->w, rec_surface->h,
		   kCVPixelFormatType_32ARGB, (CFDictionaryRef)attr, &pixel_buffer) != kCVReturnSuccess) {
		logging->out_log(LOG_ERROR, "CVPixelBufferCreate Failed.");
		return false;
	}
#endif

	// start
	BOOL rc = [asset startWriting];
	if (!rc) {
		err = [asset error];
		logging->out_log(LOG_ERROR, "AVAssetWriter startWriting Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}

	[asset startSessionAtSourceTime:kCMTimeZero];

	return true;
}

/// @attention must call this method from emu thread
void AVK_REC_VIDEO::Stop()
{
	[input markAsFinished];
	CMTime curtime = CMTimeMake((int64_t)frame_count, (int32_t)rec_fps);
	[asset endSessionAtSourceTime:curtime];

	BOOL rc = TRUE;
#if 1
	rc = [asset finishWriting];
	if (!rc) {
		NSError *err = [asset error];
		logging->out_log(LOG_ERROR, "AVAssetWriter finishWriting Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
	}
#else
	//osx 10.10 or later
	[asset finishWritingWithCompletionHandler:^{}];
	if ([asset status] == AVAssetWriterStatusFailed) {
		NSError *err = [asset error];
		logging->out_log(LOG_ERROR, "AVAssetWriter finishWriting Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		rc = FALSE;
	}
#endif

#ifdef USE_AVK_CREATE_PIXEL_BUFFER
	if (pixel_buffer) {
		CVPixelBufferRelease(pixel_buffer);
		pixel_buffer = nil;
	}
#endif

//	if (rc) emu->out_msg_x(_T("Video file was saved."));
	ReleasePool(USE_REC_VIDEO_AVKIT);
}

bool AVK_REC_VIDEO::Restart()
{
	vid->Stop();
	return vid->Start(RECORD_VIDEO_TYPE_AVKIT, -1, vid->GetSrcRect(), NULL, false);
}

/// @attention must call this method from emu thread
bool AVK_REC_VIDEO::Record()
{
	rec_surface->Lock();

	void *buffer = rec_surface->GetBuffer();

#ifdef USE_AVK_CREATE_PIXEL_BUFFER
	uint32_t buffer_size = rec_surface->GetBufferSize();

	CVPixelBufferLockBaseAddress(pixel_buffer, 0);
	void *pixel_data = CVPixelBufferGetBaseAddress(pixel_buffer);

	CGBitmapInfo bi = kCGBitmapByteOrderDefault;
	CGDataProviderRef pv = CGDataProviderCreateWithData(NULL, (const void *)buffer, buffer_size, NULL);

	CGImageRef imageref = CGImageCreate(rec_surface->Width(), rec_surface->Height(), bpc, bpp, bpr, color_space, bi, pv, NULL, false, kCGRenderingIntentDefault);


	CGContextRef context = CGBitmapContextCreate(pixel_data,
		 rec_surface->Width(),
		 rec_surface->Height(),
		 bpc,
		 bpr,
		 color_space,
		 (CGBitmapInfo)kCGImageAlphaNoneSkipFirst);

	CGContextDrawImage(context, CGRectMake(0, 0, rec_surface->Width(), rec_surface->Height()), imageref);
	CGContextRelease(context);

	CGImageRelease(imageref);
	CGDataProviderRelease(pv);

	CMTime curtime = CMTimeMake((int64_t)frame_count, (int32_t)rec_fps);
	if (![input_adaptor appendPixelBuffer:pixel_buffer withPresentationTime:curtime]) {
		// error
		logging->out_log(LOG_ERROR, "AVAssetWriteInput appendPixelBuffer Failed.");
	}
	frame_count++;

	CVPixelBufferUnlockBaseAddress(pixel_buffer, 0);

#else
	NSDictionary *attr = @{
	   (NSString *)kCVPixelBufferCGImageCompatibilityKey : [NSNumber numberWithBool:YES],
	   (NSString *)kCVPixelBufferCGBitmapContextCompatibilityKey : [NSNumber numberWithBool:YES]
   };

	CVPixelBufferRef pixel_buffer;
	if (CVPixelBufferCreateWithBytes(kCFAllocatorDefault, rec_surface->Width(), rec_surface->Height(),
			 kCVPixelFormatType_32BGRA, buffer, bpr, NULL, NULL, (CFDictionaryRef)attr, &pixel_buffer) == kCVReturnSuccess) {

		CVPixelBufferLockBaseAddress(pixel_buffer, 0);
//		void *pb = CVPixelBufferGetBaseAddress(b);

		CMTime curtime = CMTimeMake((int64_t)frame_count, (int32_t)rec_fps);
		if (![input_adaptor appendPixelBuffer:pixel_buffer withPresentationTime:curtime]) {
			// error
			logging->out_log(LOG_ERROR, "AVAssetWriteInput appendPixelBuffer Failed.");
		}
		frame_count++;

		CVPixelBufferUnlockBaseAddress(pixel_buffer, 0);
	}
#endif

	rec_surface->Unlock();

	return true;
}

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_AVKIT */
