/** @file ffm_loadlib.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ load ffmpeg library ]

	@note
	If use this, get ffmpeg library from http://www.ffmpeg.org/.
*/

#ifndef _FFM_LOAD_LIBRARY_H_
#define _FFM_LOAD_LIBRARY_H_

#include "../../rec_video_defs.h"
#include "../../common.h"

#if defined(USE_REC_VIDEO_FFMPEG) || defined(USE_REC_AUDIO_FFMPEG)

#define USE_DYNAMIC_LOADING	1

#if !defined(_WIN32) || (_MSC_VER <= 1500) || defined(__MINGW32__)
#ifndef UINT64_C
#define UINT64_C(c) c ## ULL
#endif
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
//#include <libavformat/avio.h>
};

//
// entry point of avcodec-57.dll
//
#if LIBAVCODEC_VERSION_MAJOR < 59
extern void (*f_avcodec_register_all)(void);
#endif
extern AVCodec *(*f_avcodec_find_encoder)(enum AVCodecID id);
extern int (*f_avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
//extern attribute_deprecated int (*f_avcodec_encode_video2)(AVCodecContext *avctx, AVPacket *avpkt,
//				const AVFrame *frame, int *got_packet_ptr);
//extern attribute_deprecated int (*f_avcodec_encode_audio2)(AVCodecContext *avctx, AVPacket *avpkt,
//				const AVFrame *frame, int *got_packet_ptr);
extern void (*f_av_init_packet)(AVPacket *pkt);
extern void (*f_av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src, AVRational tb_dst);
//extern attribute_deprecated void (*f_av_free_packet)(AVPacket *pkt);
extern int (*f_avcodec_fill_audio_frame)(AVFrame *frame, int nb_channels,
				enum AVSampleFormat sample_fmt, const uint8_t *buf,
				int buf_size, int align);
extern AVCodecContext *(*f_avcodec_alloc_context3)(const AVCodec *codec);
extern void (*f_avcodec_free_context)(AVCodecContext **avctx);
extern int (*f_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par);
extern int (*f_avcodec_parameters_from_context)(AVCodecParameters *par, const AVCodecContext *codec);
extern int (*f_avcodec_receive_packet)(AVCodecContext *avctx, AVPacket *avpkt);
extern int (*f_avcodec_send_frame)(AVCodecContext *avctx, const AVFrame *frame);
extern AVPacket *(*f_av_packet_alloc)(void);
extern void (*f_av_packet_unref)(AVPacket *pkt);
extern void (*f_av_packet_free)(AVPacket **pkt);

//
// entry point of avformat-57.dll
//
#if LIBAVFORMAT_VERSION_MAJOR < 59
extern void (*f_av_register_all)(void);
#endif
//extern AVFormatContext *(*f_avformat_alloc_context)(void);
extern void (*f_avformat_free_context)(AVFormatContext *s);
#if LIBAVFORMAT_VERSION_MAJOR < 59
extern AVOutputFormat *(*f_av_guess_format)(const char *short_name,
				const char *filename,
				const char *mime_type);
extern int (*f_avformat_alloc_output_context2)(AVFormatContext **ctx, AVOutputFormat *oformat,
			    const char *format_name, const char *filename);
#else
extern const AVOutputFormat *(*f_av_guess_format)(const char *short_name,
				const char *filename,
				const char *mime_type);
extern int (*f_avformat_alloc_output_context2)(AVFormatContext **ctx, const AVOutputFormat *oformat,
			    const char *format_name, const char *filename);
#endif
extern int (*f_avformat_query_codec)(const AVOutputFormat *ofmt, enum AVCodecID codec_id,
				int std_compliance);
extern AVStream *(*f_avformat_new_stream)(AVFormatContext *s, const AVCodec *c);
extern int (*f_avformat_write_header)(AVFormatContext *s, AVDictionary **options);
extern int (*f_av_write_frame)(AVFormatContext *s, AVPacket *pkt);
extern int (*f_av_write_trailer)(AVFormatContext *s);
extern int (*f_avio_open)(AVIOContext **s, const char *url, int flags);
extern int64_t (*f_avio_size)(AVIOContext *s);
extern int (*f_avio_close)(AVIOContext *s);

//
// entry point of swscale-4.dll
//
extern struct SwsContext *(*f_sws_getContext)(int srcW, int srcH, enum AVPixelFormat srcFormat,
				int dstW, int dstH, enum AVPixelFormat dstFormat,
				int flags, SwsFilter *srcFilter,
				SwsFilter *dstFilter, const double *param);
extern int (*f_sws_scale)(struct SwsContext *c, const uint8_t *const srcSlice[],
				const int srcStride[], int srcSliceY, int srcSliceH,
				uint8_t *const dst[], const int dstStride[]);
extern void (*f_sws_freeContext)(struct SwsContext *swsContext);

//
// entry point of avutil-55.dll
//
extern AVFrame *(*f_av_frame_alloc)(void);
extern void (*f_av_frame_free)(AVFrame **frame);
extern int (*f_av_image_alloc)(uint8_t *pointers[4], int linesizes[4],
				int w, int h, enum AVPixelFormat pix_fmt, int align);
extern void *(*f_av_malloc)(size_t size);
//extern void (*f_av_free)(void *ptr);
extern void (*f_av_freep)(void *ptr);
extern void (*f_av_log_set_callback)(void (*callback)(void*, int, const char*, va_list));
extern const char* (*f_av_default_item_name)(void* ctx);
extern void (*f_av_log_set_level)(int level);
extern int (*f_av_get_channel_layout_nb_channels)(uint64_t channel_layout);
//extern int (*f_av_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples,
//				enum AVSampleFormat sample_fmt, int align);
extern int (*f_av_frame_get_buffer)(AVFrame *frame, int align);

bool FFMPEG_LoadLibrary(int reffer_num);
void FFMPEG_UnloadLibrary(int reffer_num);

#endif /* USE_REC_VIDEO_FFMPEG || USE_REC_AUDIO_FFMPEG */

#endif /* _FFM_LOAD_LIBRARY_H_ */
