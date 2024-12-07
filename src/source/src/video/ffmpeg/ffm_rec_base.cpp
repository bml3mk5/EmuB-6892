/** @file ffm_rec_base.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.12.18 -

	@brief [ record video using ffmpeg ]
*/

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_FFMPEG)

#include "ffm_rec_base.h"
#include "../rec_video.h"
#include "../../emu.h"
#include "../../csurface.h"
#include "../../utility.h"
#include "../../fileio.h"

FFM_REC_BASE::FFM_REC_BASE(EMU *new_emu)
{
	emu = new_emu;

	avio = NULL;
	fmtcont = NULL;
	oformat = NULL;

	codcont = NULL;
	frame = NULL;
	srcframe = NULL;
	packet = NULL;

	write_error_count = 0;
}

FFM_REC_BASE::~FFM_REC_BASE()
{
}

void FFM_REC_BASE::Release()
{
	if (fmtcont) {
		f_avformat_free_context(fmtcont);
		fmtcont = NULL;
	}
	if (oformat) {
		f_av_freep(&oformat);
		oformat = NULL;
	}
	if (avio) {
		f_avio_close(avio);
		avio = NULL;
	}
	if (packet) {
		f_av_packet_free(&packet);
		packet = NULL;
	}
	if (srcframe) {
		f_av_frame_free(&srcframe);
		srcframe = NULL;
	}
	if (frame) {
		f_av_frame_free(&frame);
		frame = NULL;
	}
	if (codcont) {
#if LIBAVCODEC_VERSION_MAJOR >= 59
		f_avcodec_free_context(&codcont);
#endif
		codcont = NULL;
	}
}

void FFM_REC_BASE::RemoveFile()
{
	FILEIO::RemoveFile(crec_path.Get());
}

int FFM_REC_BASE::AddStream(enOutputType type, AVCodecID codec_id, const char *codec_name, AVCodec *&codec, AVStream *&stream)
{
	int ret = 0;

	// open file for output
//	CTchar crec_path(rec_path);
	ret = f_avio_open(&avio, crec_path.GetN(), AVIO_FLAG_WRITE);
	if (ret < 0) {
		logging->out_logf(LOG_ERROR, _T("avio_open failed: %d"), ret);
		return ret;
	}

	// decide output format by name or extension
	const AVOutputFormat *ofmttmp = f_av_guess_format(codec_name, NULL, NULL);
	if (!ofmttmp) {
		ofmttmp = f_av_guess_format(NULL, crec_path.GetN(), NULL);
	}
	if (!ofmttmp) {
		logging->out_logf(LOG_ERROR, _T("av_guess_format: no format found"));
		ret = -1;
		return ret;
	}

// AVOutputFormat is defined as FFOutputFormat in private class,
// and has some private members on the version more than 60.
// So, cannot allocate a new instance as AVOutputFormat.
#if LIBAVFORMAT_VERSION_MAJOR < 60
	// alloc and copy
	if (!oformat) {
		oformat = (AVOutputFormat *)f_av_malloc(sizeof(AVOutputFormat));
	}
	*oformat = *ofmttmp;

	switch(type) {
	case TYPE_VIDEO:
		// disable audio
		oformat->audio_codec = AV_CODEC_ID_NONE;
		oformat->subtitle_codec = AV_CODEC_ID_NONE;
		break;
	case TYPE_AUDIO:
		// disable video
		oformat->video_codec = AV_CODEC_ID_NONE;
		oformat->subtitle_codec = AV_CODEC_ID_NONE;
		break;
	default:
		break;
	}

	// alloc format context using output codec
	ret = f_avformat_alloc_output_context2(&fmtcont, oformat, NULL, NULL);

#else
	// alloc format context using output codec
	ret = f_avformat_alloc_output_context2(&fmtcont, ofmttmp, NULL, NULL);
#endif

	if (ret < 0) {
		logging->out_logf(LOG_ERROR, _T("avformat_alloc_output_context2 failed: %d"), ret);
		return ret;
	}
	logging->out_logf(LOG_DEBUG, _T("Output context name: \"%s\" [%s]"), fmtcont->oformat->name, fmtcont->oformat->long_name);

	// attach io context with format context
	fmtcont->pb = avio;

	// whether use this codec on specified file format
	logging->out_debug(_T("avformat_query_codec -----"));
	ret = f_avformat_query_codec(fmtcont->oformat, codec_id, FF_COMPLIANCE_NORMAL);
	if (ret <= 0) {
		logging->out_logf(LOG_ERROR, _T("avformat_query_codec failed: %d"), ret);
		ret = -1;
		return ret;
	}

	// find the specified video encoder
	logging->out_debug(_T("avcodec_find_encoder -----"));
	codec = f_avcodec_find_encoder(codec_id);
	if (!codec) {
		logging->out_logf(LOG_ERROR, _T("avcodec_find_encoder: not found codec id: %d"), codec_id);
		ret = -1;
		return ret;
	}

	// create a stream in the output file container
	logging->out_debug(_T("avformat_new_stream -----"));
	stream = f_avformat_new_stream(fmtcont, codec);
	if (!stream) {
		logging->out_log(LOG_ERROR, _T("avformat_new_stream failed."));
		ret = -1;
		return ret;
	}

#if LIBAVCODEC_VERSION_MAJOR < 59
	codcont = stream->codec;
#endif
	if (!codcont) {
		codcont = f_avcodec_alloc_context3(codec);
		if (!codcont) {
			logging->out_log(LOG_ERROR, _T("avcodec_alloc_context3 failed."));
			ret = -1;
			return ret;
		}
	}

	// allocate packet
	packet = f_av_packet_alloc();
	if (!packet) {
		logging->out_log(LOG_ERROR, _T("av_packet_alloc failed."));
		ret = -1;
		return ret;
	}

	return ret;
}

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_FFMPEG */
