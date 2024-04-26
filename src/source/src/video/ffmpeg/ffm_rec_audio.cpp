/** @file ffm_rec_audio.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio using ffmpeg ]
*/

#include "../../rec_video_defs.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_FFMPEG)

#include "ffm_rec_audio.h"
#include "../rec_audio.h"
#include "../../emu.h"
#include "../../utility.h"
#include "../../fileio.h"

FFM_REC_AUDIO::FFM_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio)
	: FFM_REC_BASE(new_emu)
{
	audio = new_audio;
	rec_rate = 0;

	store_samples[0].s = NULL;
	store_samples[1].s = NULL;
	store_sample_pos = 0;
	write_error_count = 0;
}

FFM_REC_AUDIO::~FFM_REC_AUDIO()
{
	Release();
	FFMPEG_UnloadLibrary(USE_REC_AUDIO_FFMPEG);
}

void FFM_REC_AUDIO::Release()
{
	if (fmtcont) {
		// finish and flush
		if (audio->NowRecording()) f_av_write_trailer(fmtcont);
	}
	if (store_samples[0].s) {
		store_samples[0].s = NULL;
	}
	if (store_samples[1].s) {
		store_samples[1].s = NULL;
	}

	FFM_REC_BASE::Release();
}

bool FFM_REC_AUDIO::IsEnabled()
{
	return FFMPEG_LoadLibrary(USE_REC_AUDIO_FFMPEG);
}

static const struct st_codec_ids codec_ids[] = {
	{ AV_CODEC_ID_AAC,	"mp4" /* "adts" */,	_T(".m4a") },
	{ AV_CODEC_ID_AC3,	"ac3",				_T(".aac") },
	{ AV_CODEC_ID_MP2,	"mp2",				_T(".mp2") },
	{ AV_CODEC_ID_MP3,	"mp3",				_T(".mp3") },
	{ AV_CODEC_ID_FLAC,	"flac",				_T(".flac") },
	{ AV_CODEC_ID_WMAV1,"wmav1",			_T(".wma") },
	{ AV_CODEC_ID_WMAV2,"wmav2",			_T(".wma") },
	{ AV_CODEC_ID_NONE, NULL, NULL },
};

const _TCHAR **FFM_REC_AUDIO::GetCodecList()
{
	static const _TCHAR *list[] = {
		_T("MPEG-4(AAC)"),
		_T("AC3"),
		_T("MP2"),
		_T("MP3"),
		_T("FLAC"),
		_T("WMAV1"),
		_T("WMAV2"),
		NULL };
	return list;
}

#if 0
const CMsg::Id *FFM_REC_AUDIO::GetQualityList()
{
	static const cMsg::Id list[] = {
		CMsg::Max_368Kbps,
		CMsg::High_256Kbps,
		CMsg::Normal_128Kbps,
		CMsg::Low_96Kbps,
		CMsg::Min_64Kbps,
		CMsg::End };
	return list;
}
#endif

int FFM_REC_AUDIO::SelectSampleRate(AVCodec *codec, int default_rate)
{
	const int *p;
	int best_samplerate = 0;

	if (!codec->supported_samplerates)
		return default_rate;

	p = codec->supported_samplerates;
	while (*p) {
		best_samplerate = FFMAX(*p, best_samplerate);
		p++;
	}
	return best_samplerate;
}

enum AVSampleFormat FFM_REC_AUDIO::SelectSampleFmt(AVCodec *codec)
{
	const enum AVSampleFormat reqs[] = {
		AV_SAMPLE_FMT_S16,
		AV_SAMPLE_FMT_S16P,
		AV_SAMPLE_FMT_FLTP,
		AV_SAMPLE_FMT_NONE
	};

	const enum AVSampleFormat *p = codec->sample_fmts;

	while (*p != AV_SAMPLE_FMT_NONE) {
		logging->out_debugf(_T("CheckSampleFmt: %d"), (int)*p);
		for(int i=0; reqs[i] != AV_SAMPLE_FMT_NONE; i++) {
			if (*p == reqs[i]) return reqs[i];
		}
		p++;
	}
	return *p;
}

bool FFM_REC_AUDIO::Start(_TCHAR *path, size_t path_size, int sample_rate)
{
#if 0
	const int bit_rates[] = {
		368000,	// max
		256000,	// high
		128000,	// normal
		 96000,	// low
		 64000,	// min
		0
	};
#endif

	int num = 0;
//	AVCodecID codec_id = AV_CODEC_ID_NONE;
	AVCodec *codec = NULL;
	AVStream *stream = NULL;
//	char name[64];
	int ret;

	if (path == NULL) return false;

	if (!FFMPEG_LoadLibrary(USE_REC_AUDIO_FFMPEG)) return false;

	rec_rate = sample_rate;

	num = emu->get_parami(VM::ParamRecAudioCodec);
//	codec_id = codec_ids[num].id;
//	strcpy(name, codec_ids[num].name);
	UTILITY::tcscat(path, path_size, codec_ids[num].ext);

	crec_path.Set(path);

	logging->out_logf(LOG_DEBUG, _T("FFM_REC_AUDIO::Start: %d %s"), codec_ids[num].id, codec_ids[num].name);

	// add stream
	ret = AddStream(TYPE_AUDIO, codec_ids[num].id, codec_ids[num].name, codec, stream);
	if (ret < 0) {
		goto FIN;
	}

	stream->id = fmtcont->nb_streams - 1;

	// put bit rate
//	codcont->bit_rate = bit_rates[emu->get_parami(VM::ParamRecAudioQuality)];
	switch(fmtcont->oformat->audio_codec) {
	case AV_CODEC_ID_MP2:
		codcont->bit_rate = 256000;
		break;
	default:
		codcont->bit_rate = 368000;
		break;
	}

	codcont->sample_rate = rec_rate; // SelectSampleRate(codec, rec_rate);
	codcont->sample_fmt = SelectSampleFmt(codec);

	if (codcont->sample_fmt == AV_SAMPLE_FMT_NONE) {
		logging->out_log(LOG_ERROR, _T("SelectSampleFmt failed."));
		goto FIN;
	}

	//	codcont->bits_per_raw_sample = RECORD_AUDIO_BITS_PER_SAMPLE;
	codcont->channel_layout = AV_CH_LAYOUT_STEREO;	// stereo
	codcont->channels = f_av_get_channel_layout_nb_channels(codcont->channel_layout); // maybe 2

	if (fmtcont->oformat->flags & AVFMT_GLOBALHEADER) {
		codcont->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	stream->time_base.num = 1;
	stream->time_base.den = codcont->sample_rate;

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

	frame->format = codcont->sample_fmt;
	frame->sample_rate = rec_rate;
	frame->nb_samples = codcont->frame_size;
	frame->channel_layout = codcont->channel_layout;
	frame->pts = 0;

	// allocate data buffer
	logging->out_debug(_T("av_frame_get_buffer -----"));
	ret = f_av_frame_get_buffer(frame, 0);
	if (ret < 0) {
        logging->out_logf(LOG_ERROR, _T("av_frame_get_buffer failed: %d"), ret);
		goto FIN;
	}

	switch(frame->format) {
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		store_samples[0].s = (int16_t *)frame->data[0];
		store_samples[1].s = (int16_t *)frame->data[1];
		break;
	case AV_SAMPLE_FMT_FLTP:
		store_samples[0].f = (float *)frame->data[0];
		store_samples[1].f = (float *)frame->data[1];
		break;
	}
	store_sample_pos = 0;
	write_error_count = 0;

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
	logging->out_log_x(LOG_ERROR, CMsg::Couldn_t_start_recording_audio);
	return false;
}

void FFM_REC_AUDIO::Stop()
{
	// get the delayed frames
#if 0
	int ret;
	AVPacket pkt;
	int got_output = 1;

	f_av_init_packet(&pkt);
#endif

	if (write_error_count > 0) {
		logging->out_logf(LOG_ERROR, "Record: previous error repeated %d times.", write_error_count);
		write_error_count = 0;
	}

	if (store_sample_pos < frame->nb_samples) {
		while(store_sample_pos < frame->nb_samples) {
			switch(frame->format) {
			case AV_SAMPLE_FMT_S16:
				store_samples[0].s[(store_sample_pos << 1)] = 0;
				store_samples[0].s[(store_sample_pos << 1) + 1] = 0;
				break;
			case AV_SAMPLE_FMT_S16P:
				store_samples[0].s[store_sample_pos] = 0;
				store_samples[1].s[store_sample_pos] = 0;
				break;
			case AV_SAMPLE_FMT_FLTP:
				store_samples[0].f[store_sample_pos] = 0.0f;
				store_samples[1].f[store_sample_pos] = 0.0f;
				break;
			}
			store_sample_pos++;
		}
#if 0
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;

		// fill samples and encode audio
		f_av_init_packet(&pkt);
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;
		frame->pts += store_sample_pos;

		// encode
		logging->out_debug(_T("Stop1: avcodec_encode_audio2 -----"));
		ret = f_avcodec_encode_audio2(codcont, &pkt, frame, &got_output);
		if (ret < 0) {
			logging->out_logf(LOG_ERROR,_T("Stop1: avcodec_encode_audio2: %d"), ret);
		} else if (got_output) {
			f_av_packet_rescale_ts(&pkt, codcont->time_base, fmtcont->streams[0]->time_base);
			f_av_write_frame(fmtcont, &pkt);
			f_av_free_packet(&pkt);
		}
#else
		frame->pts += store_sample_pos;

		Encoding(frame, packet);
#endif
		store_sample_pos = 0;
	}

	// flash remain frame
#if 0
	logging->out_debug(_T("Stop2: avcodec_encode_audio2 -----"));
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	got_output = 1;
	while (got_output) {
		ret = f_avcodec_encode_audio2(codcont, &pkt, NULL, &got_output);
		if (ret < 0) {
			logging->out_logf(LOG_ERROR, _T("Stop2: avcodec_encode_audio2 failed: %d"), ret);
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

bool FFM_REC_AUDIO::Restart()
{
	audio->Stop();
	return audio->Start(RECORD_AUDIO_TYPE_FFMPEG, rec_rate, false);
}

bool FFM_REC_AUDIO::Record(int32_t *buffer, int samples)
{
	bool rc = true;

#if 0
	int ret, got_output;
	AVPacket pkt;
#endif

	for(int i=0; i<(samples << 1); i+=2) {
		switch(frame->format) {
#ifdef USE_AUDIO_U8
		case AV_SAMPLE_FMT_S16:
			store_samples[0][(store_sample_pos << 1)] = (int16_t)((buffer[i] - 128) * 256);
			store_samples[0][(store_sample_pos << 1) + 1] = (int16_t)((buffer[i+1] - 128) * 256);
			break;
		case AV_SAMPLE_FMT_S16P:
			store_samples[0][store_sample_pos] = (int16_t)((buffer[i] - 128) * 256);
			store_samples[1][store_sample_pos] = (int16_t)((buffer[i+1] - 128) * 256);
			break;
#else
		case AV_SAMPLE_FMT_S16:
			store_samples[0].s[(store_sample_pos << 1)] = (int16_t)buffer[i];
			store_samples[0].s[(store_sample_pos << 1) + 1] = (int16_t)buffer[i+1];
			break;
		case AV_SAMPLE_FMT_S16P:
			store_samples[0].s[store_sample_pos] = (int16_t)buffer[i];
			store_samples[1].s[store_sample_pos] = (int16_t)buffer[i+1];
			break;
		case AV_SAMPLE_FMT_FLTP:
			store_samples[0].f[store_sample_pos] = ((float)(int16_t)buffer[i] / 32768.0f);
			store_samples[1].f[store_sample_pos] = ((float)(int16_t)buffer[i+1] / 32768.0f);
			break;
#endif
		}
		store_sample_pos++;

		if (store_sample_pos >= frame->nb_samples) {
			// fill samples and encode audio
#if 0
			f_av_init_packet(&pkt);
			pkt.data = NULL;    // packet data will be allocated by the encoder
			pkt.size = 0;
			frame->pts += store_sample_pos;

			// encode
//			logging->out_debug(_T("Record: avcodec_encode_audio2 -----"));
			ret = f_avcodec_encode_audio2(codcont, &pkt, frame, &got_output);
			if (ret < 0) {
				if (!write_error_count) logging->out_logf(LOG_ERROR,_T("Record: avcodec_encode_audio2: %d"), ret);
				write_error_count++;
				rc = false;
				break;
			} else if (got_output) {
				f_av_packet_rescale_ts(&pkt, codcont->time_base, fmtcont->streams[0]->time_base);
				f_av_write_frame(fmtcont, &pkt);
				f_av_free_packet(&pkt);
			}
#else
			frame->pts += store_sample_pos;

			rc = Encoding(frame, packet);
#endif
			store_sample_pos = 0;
		}
	}
	if (!rc) {
		audio->Stop();
	}
	return rc;
}

bool FFM_REC_AUDIO::Encoding(AVFrame *frame, AVPacket *packet)
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

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_FFMPEG */
