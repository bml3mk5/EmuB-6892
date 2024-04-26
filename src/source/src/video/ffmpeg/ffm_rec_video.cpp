/** @file ffm_rec_video.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ record video using ffmpeg ]
*/

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_FFMPEG)

#include "ffm_rec_video.h"
#include "../rec_video.h"
#include "../../emu.h"
#include "../../csurface.h"
#include "../../utility.h"
#include "../../fileio.h"

FFM_REC_VIDEO::FFM_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
	: FFM_REC_BASE(new_emu)
{
	vid = new_vid;
	rec_fps = 0;
	memset(&rec_rect, 0, sizeof(rec_rect));
	rec_surface = NULL;

	srcframe = NULL;
	sws_cont = NULL;
}

FFM_REC_VIDEO::~FFM_REC_VIDEO()
{
	Release();
	FFMPEG_UnloadLibrary(USE_REC_VIDEO_FFMPEG);
}

void FFM_REC_VIDEO::Release()
{
	if (fmtcont) {
		// finish and flush
		if (vid->NowRecording()) f_av_write_trailer(fmtcont);
	}
	if (sws_cont) {
		f_sws_freeContext(sws_cont);
		sws_cont = NULL;
	}
	if (srcframe) {
		f_av_frame_free(&srcframe);
		srcframe = NULL;
	}
	if (frame) {
		f_av_freep(&frame->data[0]);
	}

	FFM_REC_BASE::Release();
}

bool FFM_REC_VIDEO::IsEnabled()
{
	return FFMPEG_LoadLibrary(USE_REC_VIDEO_FFMPEG);
}

static const struct st_codec_ids codec_ids[] = {
//	{ AV_CODEC_ID_FFV1,			"ffm",	_T(".ffm") }, // mac ng
//	{ AV_CODEC_ID_H263,			"3gp",	_T(".3gp") }, // mac ng
	{ AV_CODEC_ID_H264,			"mp4",	_T(".m4v") }, // win ok / mac need libx264
	{ AV_CODEC_ID_MPEG4,		"mp4",	_T(".m4v") }, // win ok / mac warn quick time cannot play it.
	{ AV_CODEC_ID_MPEG2VIDEO,	"vob",	_T(".mpg") }, // win ok / mac ok
	{ AV_CODEC_ID_MJPEG,		"mjpeg",_T(".mjpg") },
//	{ AV_CODEC_ID_MSVIDEO1,		"avi",	_T(".avi") },
	{ AV_CODEC_ID_WMV1,			"asf",	_T(".wmv") }, // win ok
	{ AV_CODEC_ID_WMV2,			"wmv2",	_T(".wmv") }, // win ok
//	{ AV_CODEC_ID_MPEG1VIDEO,	"mp1",	_T(".mp1") }, // win ng
//	{ AV_CODEC_ID_RAWVIDEO,		"raw",	_T(".raw") }, // win ok
	{ AV_CODEC_ID_NONE, NULL, NULL },
};

const _TCHAR **FFM_REC_VIDEO::GetCodecList()
{
	static const _TCHAR *list[] = {
//		_T("FFmpeg video"),
//		_T("H.263"),
		_T("MPEG-4(H.264)"),
		_T("MPEG-4"),
		_T("MPEG-2"),
		_T("Motion JPEG"),
//		_T("MS Video-1"),
		_T("WMV1"),
		_T("WMV2"),
		NULL };
	return list;
}

const CMsg::Id *FFM_REC_VIDEO::GetQualityList()
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

bool FFM_REC_VIDEO::Start(_TCHAR *path, size_t path_size, int fps, const VmRectWH *srcrect, CSurface *srcsurface, bool show_dialog)
{
	const int bit_rates[] = {
		 50000000,	// max
	 	 10000000,	// high
		  1000000,	// normal
		   500000,	// low
		   100000,	// min
		0
	};

	int num = 0;
//	AVCodecID codec_id = AV_CODEC_ID_NONE;
	AVCodec *codec = NULL;
	AVStream *stream = NULL;
//	char name[64];
	int ret;

	if (path == NULL) return false;

	if (!FFMPEG_LoadLibrary(USE_REC_VIDEO_FFMPEG)) return false;

	rec_fps = fps;

	if (srcrect != NULL) {
		rec_rect = *srcrect;
	}
	if (srcsurface != NULL) {
		rec_surface = srcsurface;
	}
	if (rec_fps <= 0 || rec_rect.w <= 0 || rec_rect.h <= 0 || rec_surface == NULL) {
		return false;
	}

	num = emu->get_parami(VM::ParamRecVideoCodec);
//	codec_id = codec_ids[num].id;
//	strcpy(name, codec_ids[num].name);
	UTILITY::tcscat(path, path_size, codec_ids[num].ext);

	crec_path.Set(path);

	logging->out_logf(LOG_DEBUG, _T("FFM_REC_VIDEO::Start: %d %s"), codec_ids[num].id, codec_ids[num].name);

	// add stream
	ret = AddStream(TYPE_VIDEO, codec_ids[num].id, codec_ids[num].name, codec, stream);
	if (ret < 0) {
		goto FIN;
	}

	if (codcont->codec_id == AV_CODEC_ID_NONE) {
		codcont->codec_id = fmtcont->oformat->video_codec;
	}

	codcont->coded_width = rec_rect.w;
	codcont->coded_height = rec_rect.h;
	codcont->width = rec_rect.w;
	codcont->height = rec_rect.h;

	// frames per second
	codcont->time_base.num = 1;
	codcont->time_base.den = rec_fps;
	stream->time_base = codcont->time_base;

	codcont->gop_size = rec_fps;
	codcont->max_b_frames = 1;
	codcont->pix_fmt = AV_PIX_FMT_YUV420P;

	// put bit rate
	codcont->bit_rate = bit_rates[emu->get_parami(VM::ParamRecVideoQuality)];

	if (fmtcont->oformat->flags & AVFMT_GLOBALHEADER) {
		codcont->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	switch(fmtcont->oformat->video_codec) {
	case AV_CODEC_ID_H264:
//		av_opt_set(codcont->priv_data, "preset", "slow", 0);
		break;
	case AV_CODEC_ID_H263:
	case AV_CODEC_ID_WMV1:
	case AV_CODEC_ID_WMV2:
		codcont->max_b_frames = 0;
		break;
	case AV_CODEC_ID_MJPEG:
		codcont->pix_fmt = AV_PIX_FMT_YUVJ444P;
		codcont->max_b_frames = 0;
		break;
	case AV_CODEC_ID_MSVIDEO1:
			codcont->pix_fmt = AV_PIX_FMT_RGB555;
			break;
	case AV_CODEC_ID_RAWVIDEO:
#if defined(_RGB888)
		codcont->pix_fmt = AV_PIX_FMT_RGB32;
#elif defined(_RGB565)
		codcont->pix_fmt = AV_PIX_FMT_RGB565;
#elif defined(_RGB555)
		codcont->pix_fmt = AV_PIX_FMT_RGB555;
#endif
		break;
	default:
		break;
	}

	// open it
	logging->out_debug(_T("avcodec_open2 -----"));
	ret = f_avcodec_open2(codcont, codec, NULL);
	if (ret < 0) {
		logging->out_logf(LOG_ERROR, _T("avcodec_open2 failed: %d"), ret);
		goto FIN;
	}

	// create frame
	logging->out_debug(_T("av_frame_alloc -----"));
	frame = f_av_frame_alloc();
	if (!frame) {
		logging->out_log(LOG_ERROR, _T("av_frame_alloc failed."));
		goto FIN;
	}
	frame->format = codcont->pix_fmt;
	frame->width  = codcont->width;
	frame->height = codcont->height;
	frame->pts = 0;

//	frame->quality = 1; // FF_LAMBDA_MAX;

	// allocate data buffer
	logging->out_debug(_T("av_image_alloc -----"));
	ret = f_av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, (AVPixelFormat)frame->format, 32);
	if (ret < 0) {
		logging->out_log(LOG_ERROR, _T("av_image_alloc failed."));
		goto FIN;
	}

	// create frame for source image
	logging->out_debug(_T("av_frame_alloc -----"));
	srcframe = f_av_frame_alloc();
	if (!srcframe) {
		logging->out_log(LOG_ERROR, _T("av_frame_alloc failed."));
		goto FIN;
	}
	srcframe->width = frame->width;
	srcframe->height = frame->height;
#if defined(_RGB888)
#if defined(__APPLE__) && defined(__MACH__)
	srcframe->format = AV_PIX_FMT_BGR32;
#else
#if defined(USE_WIN)
	// windows uses RGBA
	srcframe->format = AV_PIX_FMT_RGB32;
#else
	// SDL always uses BGRA
	srcframe->format = AV_PIX_FMT_BGR32;
#endif
#endif
	srcframe->linesize[0] = frame->width * 4;
#elif defined(_RGB565)
	srcframe->format = AV_PIX_FMT_RGB565;
	srcframe->linesize[0] = frame->width * 2;
#elif defined(_RGB555)
	srcframe->format = AV_PIX_FMT_RGB555;
	srcframe->linesize[0] = frame->width * 2;
#endif

	// for convert frame image
	logging->out_debug(_T("sws_getContext -----"));
	sws_cont = f_sws_getContext(srcframe->width, srcframe->height, (AVPixelFormat)srcframe->format,
	frame->width, frame->height, (AVPixelFormat)frame->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (!sws_cont) {
		logging->out_log(LOG_ERROR, _T("sws_getContext failed."));
		goto FIN;
	}

	f_avcodec_parameters_from_context(stream->codecpar, codcont);

	// write the header of the output file container
	logging->out_debug(_T("avformat_write_header -----"));
    ret = f_avformat_write_header(fmtcont, NULL);
	if (ret < 0) {
		logging->out_logf(LOG_ERROR, _T("avformat_write_header failed: %d"), ret);
		goto FIN;
	}

	return true;
FIN:
	Release();
	RemoveFile();
	logging->out_log_x(LOG_ERROR, CMsg::Couldn_t_start_recording_video);
	return false;
}

void FFM_REC_VIDEO::Stop()
{
	// get the delayed frames
#if 0
	int ret;
	AVPacket pkt;
	f_av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	int got_output = 1;
	while (got_output) {
		ret = f_avcodec_encode_video2(codcont, &pkt, NULL, &got_output);
		if (ret < 0) {
			logging->out_logf(LOG_ERROR, _T("Stop: avcodec_encode_video2 failed: %d"), ret);
			break;
		}
		if (got_output) {
			f_av_packet_rescale_ts(&pkt, codcont->time_base, fmtcont->streams[0]->time_base);
			f_av_write_frame(fmtcont, &pkt);
			f_av_free_packet(&pkt);
		}
	}
#else
	Encoding(NULL, packet);
#endif
	Release();
}

bool FFM_REC_VIDEO::Restart()
{
	vid->Stop();
	return vid->Start(RECORD_VIDEO_TYPE_FFMPEG, -1, vid->GetSrcRect(), NULL, false);
}

bool FFM_REC_VIDEO::Record()
{
	bool rc = true;

#if 0
	int ret, got_output;
	AVPacket pkt;
#endif

	// convert frame
	rec_surface->Lock();
	srcframe->data[0] = (uint8_t *)rec_surface->GetBuffer();
    f_sws_scale(sws_cont, (uint8_t const * const *)srcframe->data, srcframe->linesize, 0, srcframe->height,
		  frame->data, frame->linesize);
	rec_surface->Unlock();

	// encode 1 frame of video
#if 0
	f_av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;

	frame->pts++;

	// encode the image
	ret = f_avcodec_encode_video2(codcont, &pkt, frame, &got_output);
	if (ret < 0) {
		logging->out_logf(LOG_ERROR,_T("Record: avcodec_encode_video2: %d"), ret);
		rc = false;
	} else if (got_output) {
		f_av_packet_rescale_ts(&pkt, codcont->time_base, fmtcont->streams[0]->time_base);
		f_av_write_frame(fmtcont, &pkt);
		f_av_free_packet(&pkt);
	}
#else
	frame->pts++;

	rc = Encoding(frame, packet);
#endif
	if (rc) {
#if defined(_M_X64) || defined(x86_64) || defined(__x86_64)
		// if file size > (2TB - 16MB), create new file
		if(f_avio_size(avio) >= 0x7fffffffff000000L)
#else
		// if file size > (2GB - 16MB), create new file
		if(f_avio_size(avio) >= 0x7f000000L)
#endif
		{
			rc = Restart();
		}
	} else {
		vid->Stop();
	}
	return rc;
}

bool FFM_REC_VIDEO::Encoding(AVFrame *frame, AVPacket *packet)
{
	bool rc = true;

	int ret = f_avcodec_send_frame(codcont, frame);
	if (ret < 0) {
		// error
		if (!write_error_count) logging->out_logf(LOG_ERROR,_T("Record: avcodec_send_frame: %d"), ret);
		write_error_count++;
		rc = false;
	}
	while(ret >= 0) {
		ret = f_avcodec_receive_packet(codcont, packet);
		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
			break;
		} else if (ret < 0) {
			// error
			if (!write_error_count) logging->out_logf(LOG_ERROR,_T("Record: avcodec_receive_packet: %d"), ret);
			write_error_count++;
			rc = false;
			break;
		}
		f_av_packet_rescale_ts(packet, codcont->time_base, fmtcont->streams[0]->time_base);
		f_av_write_frame(fmtcont, packet);
		f_av_packet_unref(packet);
	}
	return rc;
}

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_FFMPEG */
