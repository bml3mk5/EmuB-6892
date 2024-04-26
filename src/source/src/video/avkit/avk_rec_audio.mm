/** @file avk_rec_audio.mm

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.11.03 -

 @brief [ record audio using AVFoundation ]

 @note need MacOSX 10.7 or later
 */

#import "avk_rec_audio.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_AVKIT)

//#import "CoreVideo/CVPixelBuffer.h"
#import "../../depend.h"
#import "../rec_audio.h"
#import "../../emu.h"
#import "../../gui/gui.h"
#import "avk_rec_common.h"

#define SAMPLE_BUFFER_SIZE 2048

AVK_REC_AUDIO::AVK_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio)
{
	emu = new_emu;
	audio = new_audio;

	rec_rate = 0;
	frame_count = 0;
	store_sample_pos = 0;
	use_float = false;
	sample_idx = 0;
	write_error_count = 0;

	asset = nil;
//	store_buffer = NULL;
	sample_buffer[0] = NULL;
	sample_buffer[1] = NULL;
	rec_file = nil;
}

AVK_REC_AUDIO::~AVK_REC_AUDIO()
{
}

bool AVK_REC_AUDIO::IsEnabled()
{
	return true;
}

enum codec_ids {
	CODEC_M4A = 0,
	CODEC_WAV,
	CODEC_AIF,
//	CODEC_3GPP,
//	CODEC_MP3,
//	CODEC_AC3,
//	CODEC_AUA
	CODEC_UNKNOWN
};

typedef struct {
	enum codec_ids   num;
	NSString * const fil;
	FourCharCode     fmt;
	NSString * const ext;
} codType_t;

const codType_t codType[] = {
	{CODEC_M4A, AVFileTypeAppleM4A,   kAudioFormatMPEG4AAC,   @".m4a" },
	{CODEC_WAV, AVFileTypeWAVE,       kAudioFormatLinearPCM,  @".wav" },
	{CODEC_AIF, AVFileTypeAIFF,       kAudioFormatLinearPCM,  @".aif" },
//	{CODEC_3GPP,AVFileTypeAMR,        kAudioFormatAMR,        @".amr" },
//	{CODEC_MP3, AVFileTypeMPEGLayer3, kAudioFormatMPEGLayer3, @".mp3" },	// 10.9 or later
//	{CODEC_AC3, AVFileTypeAC3,        kAudioFormatAC3,        @".ac3" },	// 10.9 or later
//	{CODEC_AUA, AVFileTypeSunAU,      kAudioFormatALaw,       @".au"  },	// 10.9 or later
};

const _TCHAR **AVK_REC_AUDIO::GetCodecList()
{
	static const _TCHAR *list[] = {
		_T("M4A(AAC)"),
		_T("WAVE"),
		_T("AIFF"),
//		_T("3GPP"),
//		_T("MP3"),
//		_T("AC3"),
//		_T("AU"),
		NULL };
	return list;
}

#if 0
const CMsg::Id **AVK_REC_AUDIO::GetQualityList()
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
#endif

/// @attention must call this method from emu thread
bool AVK_REC_AUDIO::Start(const _TCHAR *path, size_t path_size, int sample_rate)
{
	AllocPool(USE_REC_AUDIO_AVKIT);

	int codTypeNum = emu->get_parami(VM::ParamRecAudioCodec);
	if (codTypeNum < 0 || CODEC_UNKNOWN <= codTypeNum) codTypeNum = 0;

//	int quoTypeNum = emu->get_parami(VM::ParamRecAudioQuality);
//	if (quoTypeNum < 0 || 4 < quoTypeNum) quoTypeNum = 0;

	if (path != NULL) {
		// add extention
		CTchar cpath(path);
		rec_file = [NSString stringWithUTF8String:cpath.GetN()];
		rec_file = [rec_file stringByAppendingString:codType[codTypeNum].ext];
	}
	rec_rate = sample_rate;
	frame_count = 0;

	if (rec_file == nil || rec_rate <= 0) {
		return false;
	}

	// create movie stream for store audio frames
	NSError *err;
	NSURL *rec_url = [NSURL fileURLWithPath:rec_file];
	asset = [AVAssetWriter assetWriterWithURL:rec_url fileType:codType[codTypeNum].fil error:&err];
	if (!asset) {
		logging->out_log(LOG_ERROR, "AVAssetWriter init Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}

	// input type
	int quoType = 0;

	// Configure the channel layout as stereo.
	AudioChannelLayout stereoChannelLayout = {
		.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo,
		.mChannelBitmap = 0,
		.mNumberChannelDescriptions = 0
	};
	// Convert the channel layout object to an NSData object.
	NSData *channelLayoutAsData = [NSData dataWithBytes:&stereoChannelLayout
		length:offsetof(AudioChannelLayout, mChannelDescriptions)];

	NSMutableDictionary *settings = [NSMutableDictionary dictionaryWithObjectsAndKeys:
		[NSNumber numberWithInt:codType[codTypeNum].fmt], AVFormatIDKey,
		[NSNumber numberWithInt:rec_rate], AVSampleRateKey,
		channelLayoutAsData, AVChannelLayoutKey,
		@2 , AVNumberOfChannelsKey,
		nil];

	switch(codType[codTypeNum].num) {
	case CODEC_M4A:
		quoType = 256000;
		[settings setObject:[NSNumber numberWithInt:quoType] forKey:AVEncoderBitRateKey];
		use_float = false;
		break;
	case CODEC_WAV:
		[settings setObject:@16 forKey:AVLinearPCMBitDepthKey];
		[settings setObject:@NO forKey:AVLinearPCMIsBigEndianKey];
		[settings setObject:@NO forKey:AVLinearPCMIsFloatKey];
		[settings setObject:@NO forKey:AVLinearPCMIsNonInterleaved];
		use_float = false;
		break;
	case CODEC_AIF:
		[settings setObject:@16 forKey:AVLinearPCMBitDepthKey];
		[settings setObject:@YES forKey:AVLinearPCMIsBigEndianKey];
		[settings setObject:@NO forKey:AVLinearPCMIsFloatKey];
		[settings setObject:@NO forKey:AVLinearPCMIsNonInterleaved];
		use_float = false;
		break;
	default:
		quoType = 256000;
		[settings setObject:[NSNumber numberWithInt:quoType] forKey:AVEncoderBitRateKey];
		use_float = false;
		break;
	}

	input = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio outputSettings:settings];
	if (![asset canAddInput:input]) {
		err = [asset error];
		logging->out_log(LOG_ERROR, "AVAssetWriter addInput Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}
	[asset addInput:input];

	// create description (interleaved lenear PCM)
	OSStatus sts;

	AudioStreamBasicDescription asbd;
	memset(&asbd, 0, sizeof(asbd));
	asbd.mSampleRate = rec_rate;
	asbd.mFormatID = kAudioFormatLinearPCM;
	if (use_float) {
		asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;	// float32
		asbd.mBitsPerChannel = 32;
	} else {
		asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;	// int16_t
		asbd.mBitsPerChannel = 16;
	}
	asbd.mChannelsPerFrame = 2;	// stereo
	asbd.mBytesPerFrame = asbd.mChannelsPerFrame * asbd.mBitsPerChannel / 8;
	asbd.mFramesPerPacket = 1;
	asbd.mBytesPerPacket = asbd.mBytesPerFrame * asbd.mFramesPerPacket;

	CMAudioFormatDescriptionRef desc = NULL;
	sts = CMAudioFormatDescriptionCreate(kCFAllocatorDefault, &asbd, sizeof(stereoChannelLayout), &stereoChannelLayout, 0, NULL, NULL, &desc);
	if (sts != 0) {
		logging->out_logf(LOG_ERROR, "CMAudioFormatDescriptionCreate Failed: %d", sts);
		return false;
	}

	// create buffer
	for(int n = 0; n < 2; n++) {
		CMBlockBufferRef store_buffer = NULL;
		sts = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
			NULL,
			asbd.mBytesPerFrame * SAMPLE_BUFFER_SIZE,
			NULL,
			NULL,
			0,
			asbd.mBytesPerFrame * SAMPLE_BUFFER_SIZE,
			0,
			&store_buffer);
		if (sts != 0) {
			logging->out_logf(LOG_ERROR, "CMBlockBufferCreateWithMemoryBlock Failed: %d", sts);
			CFRelease(desc);
			desc = NULL;
			return false;
		}
		CMBlockBufferAssureBlockMemory(store_buffer);

#if 1
		sts = CMAudioSampleBufferCreateWithPacketDescriptions(kCFAllocatorDefault,
			store_buffer,
			true,
			NULL,
			NULL,
			desc,
			SAMPLE_BUFFER_SIZE,
			CMTimeMake(0, rec_rate),
			NULL,
			&sample_buffer[n]);
		if (sts != 0) {
			logging->out_logf(LOG_ERROR, "CMAudioSampleBufferCreateWithPacketDescriptions Failed: %d", sts);
			CFRelease(store_buffer);
			store_buffer = NULL;
			CFRelease(desc);
			return false;
		}
#else
		CMSampleTimingInfo timing_info;
		timing_info.duration = CMTimeMake(1, rec_rate);
		timing_info.presentationTimeStamp = CMTimeMake(0, rec_rate);
//		timing_info.decodeTimeStamp = CMTimeMake(0, 1);

		size_t size_array = asbd.mBytesPerFrame;
		sts = CMSampleBufferCreate(kCFAllocatorDefault,
			store_buffer,			// CMBlockBufferRef
			true,					// Boolean dataReady
			NULL,					// CMSampleBufferMakeDataReadyCallback
			NULL,					// makeDataReadyRefcon
			desc,					// CMFormatDescriptionRef
			SAMPLE_BUFFER_SIZE,		// numSamples
			1,						// CMItemCount numSampleTimingEntries
			&timing_info,			// const CMSampleTimingInfo *
			1,						// CMItemCount numSampleSizeEntries
			&size_array,			// const size_t *sampleSizeArray
			&sample_buffer[n]);
		if (sts != 0) {
			// CMSampleBuffer.h kCMSampleBufferError_InvalidSampleData
			logging->out_logf(LOG_ERROR, "CMSampleBufferCreate Failed: %d", sts);
			CFRelease(store_buffer);
			store_buffer = NULL;
			CFRelease(desc);
			return false;
		}
#endif
	}

	sample_idx = 0;
	store_sample_pos = 0;
	write_error_count = 0;

#if 0
//	int len = CMBlockBufferGetDataLength(store_buffer);
//	char *pos;
//	sts = CMBlockBufferGetDataPointer(store_buffer, 0, NULL, NULL, &pos);
//	CMBlockBufferRef data = CMSampleBufferGetDataBuffer(sample_buffer);
//	CMAudioFormatDescriptionRef dr = CMSampleBufferGetFormatDescription(sample_buffer);
/*
	NSURL *inurl = [NSURL fileURLWithPath:@"sample.wav"];
	oya = [AVAsset assetWithURL:inurl];
	assetReader = [[AVAssetReader alloc] initWithAsset:oya error:&err];
	if (!assetReader) {
		logging->out_log(LOG_ERROR, "AVAssetReader initWithAsset Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}
	AVAssetTrack *autrk = [[assetReader.asset tracksWithMediaType:AVMediaTypeAudio] objectAtIndex:0];
//	NSDictionary *deco = @{
//						   AVFormatIDKey : [NSNumber numberWithUnsignedInt:kAudioFormatLinearPCM]
//						   };
	output = [AVAssetReaderTrackOutput
								   assetReaderTrackOutputWithTrack:autrk
								   outputSettings:nil];
//								   outputSettings:deco];
	if (![assetReader canAddOutput:output]) {
		err = [assetReader error];
		logging->out_log(LOG_ERROR, "AVAssetReader addOutput Failed.");
		if (err) logging->out_log(LOG_ERROR, [[err description] UTF8String]);
		return false;
	}
	[assetReader addOutput:output];
	BOOL rc;
	rc = [assetReader startReading];
 */
#endif

	// start
	BOOL rc;
	rc = [asset startWriting];
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
void AVK_REC_AUDIO::Stop()
{
	[input markAsFinished];
	CMTime curtime = CMTimeMake((int64_t)frame_count, (int32_t)rec_rate);
	[asset endSessionAtSourceTime:curtime];

	BOOL rc = TRUE;
	if (write_error_count > 0) logging->out_logf(LOG_ERROR, "appendSampleBuffer failed repeated %d times.", write_error_count);

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

	CFRelease(sample_buffer[0]);
	sample_buffer[0] = NULL;
	CFRelease(sample_buffer[1]);
	sample_buffer[1] = NULL;

	//	if (rc) emu->out_msg_x(_T("Video file was saved."));

	ReleasePool(USE_REC_AUDIO_AVKIT);
}

bool AVK_REC_AUDIO::Restart()
{
	Stop();
	return audio->Start(RECORD_AUDIO_TYPE_AVKIT, rec_rate, false);
}

/// @attention must call this method from emu thread
bool AVK_REC_AUDIO::Record(int32_t *buffer, int samples)
{
	char *p;
	for(int i=0; i<(samples << 1); i+=2) {
		CMBlockBufferRef store_buffer = CMSampleBufferGetDataBuffer(sample_buffer[sample_idx]);
		if (use_float) {
			CMBlockBufferGetDataPointer(store_buffer, store_sample_pos * sizeof(Float32) * 2, NULL, NULL, &p);
			Float32 *store_samples = (Float32 *)p;
#ifdef USE_AUDIO_U8
			store_samples[0] = ((Float32)(uint8_t)(buffer[i] - 128) / 128.0f);
			store_samples[1] = ((Float32)(uint8_t)(buffer[i+i] - 128) / 128.0f);
#else
			store_samples[0] = ((Float32)(int16_t)buffer[i] / 32768.0f);
			store_samples[1] = ((Float32)(int16_t)buffer[i+1] / 32768.0f);
#endif
		} else {
			CMBlockBufferGetDataPointer(store_buffer, store_sample_pos * sizeof(int16_t) * 2, NULL, NULL, &p);
			int16_t *store_samples = (int16_t *)p;
#ifdef USE_AUDIO_U8
			store_samples[0] = (uint8_t)((buffer[i] - 128) * 256);
			store_samples[1] = (uint8_t)((buffer[i+i] - 128) / 256);
#else
			store_samples[0] = (int16_t)buffer[i];
			store_samples[1] = (int16_t)buffer[i+1];
#endif
		}
		store_sample_pos++;

		if (store_sample_pos >= SAMPLE_BUFFER_SIZE) {
			OSStatus sts = CMSampleBufferMakeDataReady(sample_buffer[sample_idx]);
			if (sts == kCMSampleBufferError_BufferNotReady) {
				logging->out_log(LOG_ERROR, "CMSampleBufferMakeDataReady failed.");
			}

			if ([input isReadyForMoreMediaData]) {
#if 0
//				CMSampleBufferRef sbuf = [output copyNextSampleBuffer];

//				CMBlockBufferRef data; // = CMSampleBufferGetDataBuffer(sample_buffer);
//				AudioBufferList list;
//				sts = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(sample_buffer, NULL, &list, sizeof(list), NULL, NULL, 0, &data);

//				CMAudioFormatDescriptionRef dr = CMSampleBufferGetFormatDescription(sample_buffer);
//				size_t afitem_num;
//				const AudioFormatListItem *afitems = CMAudioFormatDescriptionGetFormatList(dr, &afitem_num);
//				size_t afl_num;
//				const AudioChannelLayout *afl = CMAudioFormatDescriptionGetChannelLayout(dr, &afl_num);

//				BOOL rc = [input appendSampleBuffer:sbuf];
#endif
				BOOL rc = [input appendSampleBuffer:sample_buffer[sample_idx]];
				if (!rc) {
					if (!write_error_count) logging->out_log(LOG_ERROR, "appendSampleBuffer failed.");
					write_error_count++;
				}

//				CFRelease(sbuf);
			}
			frame_count += store_sample_pos;
			store_sample_pos = 0;
			sample_idx = 1 - sample_idx;
		}
	}
	return true;
}

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_AVKIT */
