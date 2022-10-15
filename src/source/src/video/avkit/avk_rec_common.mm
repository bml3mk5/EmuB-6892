/** @file avk_rec_common.mm

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.11.03 -

 @brief [ record common using AVFoundation ]
 */

#import "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO_QTKIT) || defined(USE_REC_VIDEO_AVKIT) || defined(USE_REC_AUDIO_AVKIT)

#import "avk_rec_common.h"
#import <Cocoa/Cocoa.h>

NSAutoreleasePool *pool = nil;
int reffer = 0;

void AllocPool(int num)
{
	if (reffer == 0) {
		if (pool == nil) {
			pool = [[NSAutoreleasePool alloc] init];
		}
	}
	reffer |= num;
}

void ReleasePool(int num)
{
	reffer &= ~num;
	if (reffer == 0) {
		if (pool) {
			// auto release executed on emu thread. otherwise occur exception
			[pool release];
			pool = nil;
		}
	}
}

#endif /* USE_REC_VIDEO_QTKIT || USE_REC_VIDEO_AVKIT || defined(USE_REC_AUDIO_AVKIT */
