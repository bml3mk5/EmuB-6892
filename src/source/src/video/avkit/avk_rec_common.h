/** @file avk_rec_common.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.11.03 -

 @brief [ record common using AVFoundation ]
 */

#ifndef AVK_RECORD_COMMON_H
#define AVK_RECORD_COMMON_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO_QTKIT) || defined(USE_REC_VIDEO_AVKIT) || defined(USE_REC_AUDIO_AVKIT)

#include "../../common.h"

void AllocPool(int num);
void ReleasePool(int num);

#endif /* USE_REC_VIDEO_QTKIT || USE_REC_VIDEO_AVKIT || defined(USE_REC_AUDIO_AVKIT */

#endif /* AVK_RECORD_COMMON_H */
