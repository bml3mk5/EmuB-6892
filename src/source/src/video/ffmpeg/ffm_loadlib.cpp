/** @file ffm_loadlib.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ load ffmpeg library ]
*/

#include "ffm_loadlib.h"

#if defined(USE_REC_VIDEO_FFMPEG) || defined(USE_REC_AUDIO_FFMPEG)

#include "../../emu.h"

extern EMU *emu;

#include "../../loadlibrary.h"
#include "../../utility.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifndef USE_DYNAMIC_LOADING
extern "C" {
//#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
};

#ifdef _MSC_VER
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avformat.lib")
#endif
#endif

#if defined(_WIN32)
#include <windows.h>
// for load dll library
static HMODULE hAVCodec = NULL;
static HMODULE hAVFormat = NULL;
static HMODULE hSWScale = NULL;
static HMODULE hAVUtil = NULL;
#else
static void *hAVCodec = NULL;
static void *hAVFormat = NULL;
static void *hSWScale = NULL;
static void *hAVUtil = NULL;
#endif

static int library_selnum = -1;
static int reffer = 0;

#ifdef USE_DYNAMIC_LOADING

//
// entry point of avcodec-57.dll
//
#if LIBAVCODEC_VERSION_MAJOR < 59
void (*f_avcodec_register_all)(void) = NULL;
#endif
AVCodec *(*f_avcodec_find_encoder)(enum AVCodecID id) = NULL;
int (*f_avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options) = NULL;
//int (*f_avcodec_encode_video2)(AVCodecContext *avctx, AVPacket *avpkt,
//                          const AVFrame *frame, int *got_packet_ptr) = NULL;
//int (*f_avcodec_encode_audio2)(AVCodecContext *avctx, AVPacket *avpkt,
//                          const AVFrame *frame, int *got_packet_ptr) = NULL;
void (*f_av_init_packet)(AVPacket *pkt) = NULL;
void (*f_av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src, AVRational tb_dst) = NULL;
//void (*f_av_free_packet)(AVPacket *pkt) = NULL;
int (*f_avcodec_fill_audio_frame)(AVFrame *frame, int nb_channels,
							enum AVSampleFormat sample_fmt, const uint8_t *buf,
							int buf_size, int align) = NULL;
AVCodecContext *(*f_avcodec_alloc_context3)(const AVCodec *codec) = NULL;
void (*f_avcodec_free_context)(AVCodecContext **avctx) = NULL;
int (*f_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par) = NULL;
int (*f_avcodec_parameters_from_context)(AVCodecParameters *par, const AVCodecContext *codec) = NULL;
int (*f_avcodec_receive_packet)(AVCodecContext *avctx, AVPacket *avpkt) = NULL;
int (*f_avcodec_send_frame)(AVCodecContext *avctx, const AVFrame *frame) = NULL;
AVPacket *(*f_av_packet_alloc)(void) = NULL;
void (*f_av_packet_unref)(AVPacket *pkt) = NULL;
void (*f_av_packet_free)(AVPacket **pkt) = NULL;
unsigned (*f_avcodec_version)(void) = NULL;

//
// entry point of avformat-57.dll
//
#if LIBAVFORMAT_VERSION_MAJOR < 59
void (*f_av_register_all)(void) = NULL;
#endif
//AVFormatContext *(*f_avformat_alloc_context)(void) = NULL;
void (*f_avformat_free_context)(AVFormatContext *s) = NULL;
#if LIBAVFORMAT_VERSION_MAJOR < 59
AVOutputFormat *(*f_av_guess_format)(const char *short_name,
				const char *filename,
				const char *mime_type) = NULL;
int (*f_avformat_alloc_output_context2)(AVFormatContext **ctx, AVOutputFormat *oformat,
			    const char *format_name, const char *filename) = NULL;
#else
const AVOutputFormat *(*f_av_guess_format)(const char *short_name,
				const char *filename,
				const char *mime_type) = NULL;
int (*f_avformat_alloc_output_context2)(AVFormatContext **ctx, const AVOutputFormat *oformat,
			    const char *format_name, const char *filename) = NULL;
#endif
int (*f_avformat_query_codec)(const AVOutputFormat *ofmt, enum AVCodecID codec_id,
				int std_compliance) = NULL;
AVStream *(*f_avformat_new_stream)(AVFormatContext *s, const AVCodec *c) = NULL;
int (*f_avformat_write_header)(AVFormatContext *s, AVDictionary **options) = NULL;
int (*f_av_write_frame)(AVFormatContext *s, AVPacket *pkt) = NULL;
int (*f_av_write_trailer)(AVFormatContext *s) = NULL;
int (*f_avio_open)(AVIOContext **s, const char *url, int flags) = NULL;
int64_t (*f_avio_size)(AVIOContext *s) = NULL;
int (*f_avio_close)(AVIOContext *s) = NULL;
unsigned (*f_avformat_version)(void) = NULL;

//
// entry point of swscale-4.dll
//
struct SwsContext *(*f_sws_getContext)(int srcW, int srcH, enum AVPixelFormat srcFormat,
				int dstW, int dstH, enum AVPixelFormat dstFormat,
				int flags, SwsFilter *srcFilter,
				SwsFilter *dstFilter, const double *param) = NULL;
int (*f_sws_scale)(struct SwsContext *c, const uint8_t *const srcSlice[],
				const int srcStride[], int srcSliceY, int srcSliceH,
				uint8_t *const dst[], const int dstStride[]) = NULL;
void (*f_sws_freeContext)(struct SwsContext *swsContext) = NULL;
unsigned (*f_swscale_version)(void) = NULL;

//
// entry point of avutil-55.dll
//
AVFrame *(*f_av_frame_alloc)(void) = NULL;
void (*f_av_frame_free)(AVFrame **frame) = NULL;
int (*f_av_image_alloc)(uint8_t *pointers[4], int linesizes[4],
                   int w, int h, enum AVPixelFormat pix_fmt, int align) = NULL;
void *(*f_av_malloc)(size_t size) = NULL;
//void (*f_av_free)(void *ptr) = NULL;
void (*f_av_freep)(void *ptr) = NULL;
void (*f_av_log_set_callback)(void (*callback)(void*, int, const char*, va_list)) = NULL;
const char* (*f_av_default_item_name)(void* ctx) = NULL;
void (*f_av_log_set_level)(int level) = NULL;
#if LIBAVUTIL_VERSION_MAJOR < 57
int (*f_av_get_channel_layout_nb_channels)(uint64_t channel_layout) = NULL;
#endif
//int (*f_av_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples,
//				enum AVSampleFormat sample_fmt, int align) = NULL;
int (*f_av_frame_get_buffer)(AVFrame *frame, int align) = NULL;
unsigned (*f_avutil_version)(void) = NULL;
#if LIBAVUTIL_VERSION_MAJOR >= 57
int (*f_av_channel_layout_from_mask)(AVChannelLayout *channel_layout, uint64_t mask) = NULL;
int (*f_av_channel_layout_copy)(AVChannelLayout *dst, const AVChannelLayout *src) = NULL;
#endif

#else

//
// entry point of avcodec-57.dll
//
#if LIBAVCODEC_VERSION_MAJOR < 59
void (*f_avcodec_register_all)(void) = avcodec_register_all;
#endif
AVCodec *(*f_avcodec_find_encoder)(enum AVCodecID id) = avcodec_find_encoder;
int (*f_avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options) = avcodec_open2;
//int (*f_avcodec_encode_video2)(AVCodecContext *avctx, AVPacket *avpkt,
//                          const AVFrame *frame, int *got_packet_ptr) = avcodec_encode_video2;
//int (*f_avcodec_encode_audio2)(AVCodecContext *avctx, AVPacket *avpkt,
//                          const AVFrame *frame, int *got_packet_ptr) = avcodec_encode_audio2;
void (*f_av_init_packet)(AVPacket *pkt) = av_init_packet;
void (*f_av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src, AVRational tb_dst) = av_packet_rescale_ts;
//void (*f_av_free_packet)(AVPacket *pkt) = av_free_packet;
int (*f_avcodec_fill_audio_frame)(AVFrame *frame, int nb_channels,
							enum AVSampleFormat sample_fmt, const uint8_t *buf,
							int buf_size, int align) = avcodec_fill_audio_frame;
AVCodecContext *(*f_avcodec_alloc_context3)(const AVCodec *codec) = avcodec_alloc_context3;
void (*f_avcodec_free_context)(AVCodecContext **avctx) = avcodec_free_context;
int (*f_avcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par) = avcodec_parameters_to_context;
int (*f_avcodec_parameters_from_context)(AVCodecParameters *par, const AVCodecContext *codec) = avcodec_parameters_from_context;
int (*f_avcodec_receive_packet)(AVCodecContext *avctx, AVPacket *avpkt) = avcodec_receive_packet;
int (*f_avcodec_send_frame)(AVCodecContext *avctx, const AVFrame *frame) = avcodec_send_frame;
AVPacket *(*f_av_packet_alloc)(void) = av_packet_alloc;
void (*f_av_packet_unref)(AVPacket *pkt) = av_packet_unref;
void (*f_av_packet_free)(AVPacket **pkt) = av_packet_free;
unsigned (*f_avcodec_version)(void) = avcodec_version;

//
// entry point of avformat-57.dll
//
#if LIBAVFORMAT_VERSION_MAJOR < 59
void (*f_av_register_all)(void) = av_register_all;
#endif
//AVFormatContext *(*f_avformat_alloc_context)(void) = avformat_alloc_context;
void (*f_avformat_free_context)(AVFormatContext *s) = avformat_free_context;
#if LIBAVFORMAT_VERSION_MAJOR < 59
AVOutputFormat *(*f_av_guess_format)(const char *short_name,
				const char *filename,
				const char *mime_type) = av_guess_format;
int (*f_avformat_alloc_output_context2)(AVFormatContext **ctx, AVOutputFormat *oformat,
			    const char *format_name, const char *filename) = avformat_alloc_output_context2;
#else
const AVOutputFormat *(*f_av_guess_format)(const char *short_name,
				const char *filename,
				const char *mime_type) = av_guess_format;
int (*f_avformat_alloc_output_context2)(AVFormatContext **ctx, const AVOutputFormat *oformat,
			    const char *format_name, const char *filename) = avformat_alloc_output_context2;
#endif
int (*f_avformat_query_codec)(const AVOutputFormat *ofmt, enum AVCodecID codec_id,
				int std_compliance) = avformat_query_codec;
AVStream *(*f_avformat_new_stream)(AVFormatContext *s, const AVCodec *c) = avformat_new_stream;
int (*f_avformat_write_header)(AVFormatContext *s, AVDictionary **options) = avformat_write_header;
int (*f_av_write_frame)(AVFormatContext *s, AVPacket *pkt) = av_write_frame;
int (*f_av_write_trailer)(AVFormatContext *s) = av_write_trailer;
int (*f_avio_open)(AVIOContext **s, const char *url, int flags) = avio_open;
int64_t (*f_avio_size)(AVIOContext *s) = avio_size;
int (*f_avio_close)(AVIOContext *s) = avio_close;
unsigned (*f_avformat_version)(void) = avformat_version;

//
// entry point of swscale-4.dll
//
struct SwsContext *(*f_sws_getContext)(int srcW, int srcH, enum AVPixelFormat srcFormat,
				int dstW, int dstH, enum AVPixelFormat dstFormat,
				int flags, SwsFilter *srcFilter,
				SwsFilter *dstFilter, const double *param) = sws_getContext;
int (*f_sws_scale)(struct SwsContext *c, const uint8_t *const srcSlice[],
				const int srcStride[], int srcSliceY, int srcSliceH,
				uint8_t *const dst[], const int dstStride[]) = sws_scale;
void (*f_sws_freeContext)(struct SwsContext *swsContext) = sws_freeContext;
unsigned (*f_swscale_version)(void) = swscale_version;

//
// entry point of avutil-55.dll
//
AVFrame *(*f_av_frame_alloc)(void) = av_frame_alloc;
void (*f_av_frame_free)(AVFrame **frame) = av_frame_free;
int (*f_av_image_alloc)(uint8_t *pointers[4], int linesizes[4],
                   int w, int h, enum AVPixelFormat pix_fmt, int align) = av_image_alloc;
void *(*f_av_malloc)(size_t size) = av_malloc;
//void (*f_av_free)(void *ptr) = av_free;
void (*f_av_freep)(void *ptr) = av_freep;
void (*f_av_log_set_callback)(void (*callback)(void*, int, const char*, va_list)) = av_log_set_callback;
const char* (*f_av_default_item_name)(void* ctx) = av_default_item_name;
void (*f_av_log_set_level)(int level) = av_log_set_level;
#if LIBAVUTIL_VERSION_MAJOR < 57
int (*f_av_get_channel_layout_nb_channels)(uint64_t channel_layout) = av_get_channel_layout_nb_channels;
#endif
//int (*f_av_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples,
//				enum AVSampleFormat sample_fmt, int align) = av_samples_get_buffer_size;
int (*f_av_frame_get_buffer)(AVFrame *frame, int align) = av_frame_get_buffer;
unsigned (*f_avutil_version)(void) = avutil_version;
#if LIBAVUTIL_VERSION_MAJOR >= 57
int (*f_av_channel_layout_from_mask)(AVChannelLayout *channel_layout, uint64_t mask) = av_channel_layout_from_mask;
int (*f_av_channel_layout_copy)(AVChannelLayout *dst, const AVChannelLayout *src) = av_channel_layout_copy;
#endif

#endif

#ifdef _DEBUG
void cb_log(void *avcl, int level, const char *fmt, va_list vl)
{
	if (f_av_default_item_name && avcl) {
		const char *name = f_av_default_item_name(avcl);
		if (name) {
			char slevel[8];
			switch(level) {
			case AV_LOG_FATAL:
				UTILITY::strcpy(slevel,sizeof(slevel),"FATAL");
				break;
			case AV_LOG_ERROR:
				UTILITY::strcpy(slevel,sizeof(slevel),"ERROR");
				break;
			case AV_LOG_WARNING:
				UTILITY::strcpy(slevel,sizeof(slevel),"WARNING");
				break;
			case AV_LOG_INFO:
				UTILITY::strcpy(slevel,sizeof(slevel),"INFO");
				break;
			case AV_LOG_VERBOSE:
				UTILITY::strcpy(slevel,sizeof(slevel),"VERBOSE");
				break;
			case AV_LOG_DEBUG:
				UTILITY::strcpy(slevel,sizeof(slevel),"DEBUG");
				break;
			default:
				UTILITY::sprintf(slevel,sizeof(slevel),"%d",level);
				break;
			}
			logging->out_logf(LOG_DEBUG, _T("ffmpeg: %s level: %s"), name, slevel);
		}
	}
	logging->out_logv(LOG_DEBUG, fmt, vl);
}
#endif

static const struct {
	bool	end;
	int		avutil;
	int		swresample;
	int		avcodec;
	int		avformat;
	int		swscale;
} dllvers[] = {
	{ false, LIBAVUTIL_VERSION_MAJOR, LIBSWRESAMPLE_VERSION_MAJOR, LIBAVCODEC_VERSION_MAJOR, LIBAVFORMAT_VERSION_MAJOR, LIBSWSCALE_VERSION_MAJOR },
//	{ false, 59, 5, 61, 61, 8 },	// Ver. 7.1
//	{ false, 58, 4, 60, 60, 7 },	// Ver. 6.1.1
//	{ false, 57, 4, 59, 59, 6 },	// Ver. 5.1.2
//	{ false, 56, 3, 58, 58, 5 },	// Ver. 4.4.1
//	{ false, 55, 2, 57, 57, 4 },	// Ver. 3.4.1
	{ false, 0, 0, 0, 0, 0 },
	{ true,  0, 0, 0, 0, 0 },
};

enum en_ffmpeg_version_min {
	LIBAVUTIL_VERSION_MAJOR_MIN = LIBAVUTIL_VERSION_MAJOR,
	LIBAVCODEC_VERSION_MAJOR_MIN = LIBAVCODEC_VERSION_MAJOR,
	LIBAVFORMAT_VERSION_MAJOR_MIN = LIBAVFORMAT_VERSION_MAJOR,
	LIBSWSCALE_VERSION_MAJOR_MIN = LIBSWSCALE_VERSION_MAJOR,
};

bool FFMPEG_LoadLibrary(int reffer_num)
{
	if (library_selnum >= 0) goto SUCCESS;

	for(int i=0; dllvers[i].end != true; i++) {
		// Set entry points of AVUtil function
		LOAD_LIB(hAVUtil, "avutil", dllvers[i].avutil);
		GET_ADDR(f_av_frame_alloc, AVFrame *(*)(void), hAVUtil, "av_frame_alloc");
		GET_ADDR(f_av_frame_free, void (*)(AVFrame **), hAVUtil, "av_frame_free");
		GET_ADDR(f_av_image_alloc, int (*)(uint8_t *[4], int [4], int, int, enum AVPixelFormat, int), hAVUtil, "av_image_alloc");
		GET_ADDR(f_av_malloc, void *(*)(size_t), hAVUtil, "av_malloc");
//		GET_ADDR(f_av_free, void (*)(void *), hAVUtil, "av_free");
		GET_ADDR(f_av_freep, void (*)(void *), hAVUtil, "av_freep");
		GET_ADDR(f_av_log_set_callback, void (*)(void (*callback)(void*, int, const char*, va_list)), hAVUtil, "av_log_set_callback");
		GET_ADDR_OPTIONAL(f_av_default_item_name, const char* (*)(void*), hAVUtil, "av_default_item_name");
		GET_ADDR_OPTIONAL(f_av_log_set_level, void (*)(int), hAVUtil, "av_log_set_level");
#if LIBAVUTIL_VERSION_MAJOR < 57
		GET_ADDR(f_av_get_channel_layout_nb_channels, int(*)(uint64_t), hAVUtil, "av_get_channel_layout_nb_channels");
#endif
//		GET_ADDR(f_av_samples_get_buffer_size, int (*)(int *, int, int,	enum AVSampleFormat, int), hAVUtil, "av_samples_get_buffer_size");
		GET_ADDR(f_av_frame_get_buffer, int (*)(AVFrame *, int), hAVUtil, "av_frame_get_buffer");
		GET_ADDR(f_avutil_version, unsigned (*)(void), hAVUtil, "avutil_version");
#if LIBAVUTIL_VERSION_MAJOR >= 57
		GET_ADDR(f_av_channel_layout_from_mask, int (*)(AVChannelLayout *, uint64_t), hAVUtil, "av_channel_layout_from_mask");
		GET_ADDR(f_av_channel_layout_copy, int (*)(AVChannelLayout *, const AVChannelLayout *), hAVUtil, "av_channel_layout_copy");
#endif

		CHECK_VERSION(AV_VERSION_MAJOR(f_avutil_version()), LIBAVUTIL_VERSION_MAJOR_MIN, "avutil");

		// Set entry points of SWResample function (reference from avcodec)
		LOAD_LIB(hSWScale, "swresample", dllvers[i].swresample);

		// Set entry points of AVCodec function
		LOAD_LIB(hAVCodec, "avcodec", dllvers[i].avcodec);
#if LIBAVCODEC_VERSION_MAJOR < 59
		GET_ADDR(f_avcodec_register_all, void (*)(void), hAVCodec, "avcodec_register_all");
#endif
		GET_ADDR(f_avcodec_find_encoder, AVCodec *(*)(enum AVCodecID id), hAVCodec, "avcodec_find_encoder");
		GET_ADDR(f_avcodec_open2, int (*)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options), hAVCodec, "avcodec_open2");
//		GET_ADDR(f_avcodec_encode_video2, int (*)(AVCodecContext *, AVPacket *, const AVFrame *, int *), hAVCodec, "avcodec_encode_video2");
//		GET_ADDR(f_avcodec_encode_audio2, int (*)(AVCodecContext *, AVPacket *, const AVFrame *, int *), hAVCodec, "avcodec_encode_audio2");
		GET_ADDR(f_av_init_packet, void (*)(AVPacket *pkt), hAVCodec, "av_init_packet");
		GET_ADDR(f_av_packet_rescale_ts, void (*)(AVPacket *pkt, AVRational tb_src, AVRational tb_dst), hAVCodec, "av_packet_rescale_ts");
//		GET_ADDR(f_av_free_packet, void (*)(AVPacket *pkt), hAVCodec, "av_free_packet");
		GET_ADDR(f_avcodec_fill_audio_frame, int (*)(AVFrame *, int, enum AVSampleFormat, const uint8_t *, int, int), hAVCodec, "avcodec_fill_audio_frame");
		GET_ADDR(f_avcodec_alloc_context3, AVCodecContext *(*)(const AVCodec *codec), hAVCodec, "avcodec_alloc_context3");
		GET_ADDR(f_avcodec_free_context, void (*)(AVCodecContext **avctx), hAVCodec, "avcodec_free_context");
		GET_ADDR(f_avcodec_parameters_to_context, int (*)(AVCodecContext *codec, const AVCodecParameters *par), hAVCodec, "avcodec_parameters_to_context");
		GET_ADDR(f_avcodec_parameters_from_context, int (*)(AVCodecParameters *par, const AVCodecContext *codec), hAVCodec, "avcodec_parameters_from_context");
		GET_ADDR(f_avcodec_receive_packet, int (*)(AVCodecContext *avctx, AVPacket *avpkt), hAVCodec, "avcodec_receive_packet");
		GET_ADDR(f_avcodec_send_frame, int (*)(AVCodecContext *avctx, const AVFrame *frame), hAVCodec, "avcodec_send_frame");
		GET_ADDR(f_av_packet_alloc, AVPacket *(*)(void), hAVCodec, "av_packet_alloc");
		GET_ADDR(f_av_packet_unref, void (*)(AVPacket *pkt), hAVCodec, "av_packet_unref");
		GET_ADDR(f_av_packet_free, void (*)(AVPacket **pkt), hAVCodec, "av_packet_free");
		GET_ADDR(f_avcodec_version, unsigned (*)(void), hAVCodec, "avcodec_version");

		CHECK_VERSION(AV_VERSION_MAJOR(f_avcodec_version()), LIBAVCODEC_VERSION_MAJOR_MIN, "avcodec");

		// Set entry points of AVFormat function
		LOAD_LIB(hAVFormat, "avformat", dllvers[i].avformat);
#if LIBAVFORMAT_VERSION_MAJOR < 59
		GET_ADDR(f_av_register_all, void (*)(void), hAVFormat, "av_register_all");
#endif
//		GET_ADDR(f_avformat_alloc_context, AVFormatContext *(*)(void), hAVFormat, "avformat_alloc_context");
		GET_ADDR(f_avformat_free_context, void (*)(AVFormatContext *s), hAVFormat, "avformat_free_context");
#if LIBAVFORMAT_VERSION_MAJOR < 59
		GET_ADDR(f_av_guess_format, AVOutputFormat *(*)(const char *, const char *, const char *), hAVFormat, "av_guess_format");
		GET_ADDR(f_avformat_alloc_output_context2, int(*)(AVFormatContext **, AVOutputFormat *, const char *, const char *), hAVFormat, "avformat_alloc_output_context2");
#else
		GET_ADDR(f_av_guess_format, const AVOutputFormat *(*)(const char *, const char *, const char *), hAVFormat, "av_guess_format");
		GET_ADDR(f_avformat_alloc_output_context2, int(*)(AVFormatContext **, const AVOutputFormat *, const char *, const char *), hAVFormat, "avformat_alloc_output_context2");
#endif
		GET_ADDR(f_avformat_query_codec, int (*)(const AVOutputFormat *, enum AVCodecID, int), hAVFormat, "avformat_query_codec");
		GET_ADDR(f_avformat_new_stream, AVStream *(*)(AVFormatContext *s, const AVCodec *c), hAVFormat, "avformat_new_stream");
		GET_ADDR(f_avformat_write_header, int (*)(AVFormatContext *s, AVDictionary **options), hAVFormat, "avformat_write_header");
		GET_ADDR(f_av_write_frame, int (*)(AVFormatContext *s, AVPacket *pkt), hAVFormat, "av_write_frame");
		GET_ADDR(f_av_write_trailer, int (*)(AVFormatContext *s), hAVFormat, "av_write_trailer");
		GET_ADDR(f_avio_open, int (*)(AVIOContext **s, const char *url, int flags), hAVFormat, "avio_open");
		GET_ADDR(f_avio_size, int64_t (*)(AVIOContext *s), hAVFormat, "avio_size");
		GET_ADDR(f_avio_close, int (*)(AVIOContext *s), hAVFormat, "avio_close");
		GET_ADDR(f_avformat_version, unsigned (*)(void), hAVFormat, "avformat_version");

		CHECK_VERSION(AV_VERSION_MAJOR(f_avformat_version()), LIBAVFORMAT_VERSION_MAJOR_MIN, "avformat");

		// Set entry points of SWScale function
		LOAD_LIB(hSWScale, "swscale", dllvers[i].swscale);
		GET_ADDR(f_sws_getContext, struct SwsContext *(*)(int srcW, int srcH, enum AVPixelFormat srcFormat,
												   int dstW, int dstH, enum AVPixelFormat dstFormat,
												   int flags, SwsFilter *srcFilter,
												   SwsFilter *dstFilter, const double *param), hSWScale, "sws_getContext");
		GET_ADDR(f_sws_scale, int (*)(struct SwsContext *c, const uint8_t *const srcSlice[],
							   const int srcStride[], int srcSliceY, int srcSliceH,
							   uint8_t *const dst[], const int dstStride[]), hSWScale, "sws_scale");
		GET_ADDR(f_sws_freeContext, void (*)(struct SwsContext *swsContext), hSWScale, "sws_freeContext");
		GET_ADDR(f_swscale_version, unsigned (*)(void), hSWScale, "swscale_version");

		CHECK_VERSION(AV_VERSION_MAJOR(f_swscale_version()), LIBSWSCALE_VERSION_MAJOR_MIN, "swscale");

		//

		library_selnum = i;
		break;
	}
	if (library_selnum < 0) goto FAILED;

	// register all the codecs
#if LIBAVFORMAT_VERSION_MAJOR < 59
	f_av_register_all();
#endif
#if LIBAVCODEC_VERSION_MAJOR < 59
    f_avcodec_register_all();
#endif

#ifdef _DEBUG
	f_av_log_set_callback(cb_log);
#else
	if (f_av_log_set_level) f_av_log_set_level(AV_LOG_INFO);
#endif
SUCCESS:
	reffer |= reffer_num;
	return true;
FAILED:
//	logging->out_log(LOG_ERROR, _T("Couldn't load FFmpeg library."));
	return false;
}

void FFMPEG_UnloadLibrary(int reffer_num)
{
	reffer &= ~reffer_num;
	if (reffer) return;

	UNLOAD_LIB(hAVCodec);
	UNLOAD_LIB(hAVFormat);
	UNLOAD_LIB(hSWScale);
	UNLOAD_LIB(hAVUtil);
	library_selnum = -1;
}

#endif /* USE_REC_VIDEO_FFMPEG || USE_REC_AUDIO_FFMPEG */
