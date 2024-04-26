/** @file mmf_rec_audio.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ record audio using microsoft media foundation ]

	@note This component can use on Windows7 or later.
*/

#include "mmf_rec_audio.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_MMF)

//#define USE_MMF_AUDIO_WMA

#ifdef USE_MMF_AUDIO_WMA
#include <Wmcodecdsp.h>
#endif

#include "../rec_audio.h"
#include "../../emu.h"
#include "../../utility.h"

MMF_REC_AUDIO::MMF_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio)
{
	emu = new_emu;
	audio = new_audio;
	rec_rate = 0;
	rec_path = NULL;

	streamIndex = 0;
	sample_time = 0;
	frame_duration = 0;
	bytes_per_sample = 0;
	write_error_count = 0;
	sample_buffer_size = 0;
//	sample_is_multiple = 0;

	sinkWriter = NULL;
	for(int n=0; n<BUFFER_COUNT; n++) {
		mediaBuffers[n] = NULL;
		mediaSamples[n] = NULL;
	}
	buffer_idx = 0;
}

MMF_REC_AUDIO::~MMF_REC_AUDIO()
{
	Release();
	MMF_UnloadLibrary(USE_REC_AUDIO_MMF);
}

#define SafeRelease(object) { if (object) object->Release(); object = NULL; }

void MMF_REC_AUDIO::Release()
{
	for(int n=0; n<BUFFER_COUNT; n++) {
		SafeRelease(mediaSamples[n]);
		SafeRelease(mediaBuffers[n]);
	}
	SafeRelease(sinkWriter);
}

void MMF_REC_AUDIO::RemoveFile()
{
#if defined(_WIN32)
	DeleteFile(rec_path);
#else
	unlink(rec_path);
#endif
}

bool MMF_REC_AUDIO::IsEnabled()
{
	return MMF_LoadLibrary(USE_REC_AUDIO_MMF);
}

enum codec_ids {
	CODEC_AAC = 0,
	CODEC_AAC4,
	CODEC_ADTS,
#ifdef USE_MMF_AUDIO_WMA
	CODEC_WMAV2,
	CODEC_WMAV3,
#endif
	CODEC_WAVE,
	CODEC_UNKNOWN
};

/// supported codec table of microsoft media foundation
typedef struct {
	enum codec_ids num;
	const GUID     cod;
	const GUID     cnt;		
	const _TCHAR  *ext;
} codTypes_t;

static const codTypes_t codTypes[] = {
	{ CODEC_AAC,  MFAudioFormat_AAC, MFTranscodeContainerType_MPEG4, _T(".m4a") },
	{ CODEC_AAC4, MFAudioFormat_AAC, MFTranscodeContainerType_MPEG4, _T(".m4a") },
//	{ CODEC_ADTS, MFAudioFormat_AAC, {0, 0, 0, 0 }, _T(".aac") },
#ifdef USE_MMF_AUDIO_WMA
	{ CODEC_WMAV2, MFAudioFormat_WMAudioV8, MFTranscodeContainerType_ASF, _T(".wma") },
	{ CODEC_WMAV3, MFAudioFormat_WMAudioV9, MFTranscodeContainerType_ASF, _T(".wma") },
#endif
	{ CODEC_WAVE, MFAudioFormat_PCM, {0, 0, 0, 0 }, _T(".wav") },
};

const _TCHAR **MMF_REC_AUDIO::GetCodecList()
{
	static const _TCHAR *list[] = {
		_T("MPEG-4(AAC@L2)"),
		_T("MPEG-4(AAC@L4)"),
//		_T("ADTS"),
#ifdef USE_MMF_AUDIO_WMA
		_T("WMAudioV8"),
		_T("WMAudioV9"),
#endif
		_T("WAVE"),
		NULL };
	return list;
}

#if 0
const CMsg::Id *MMF_REC_AUDIO::GetQualityList()
{
	static const CMsg::Id list[] = {
		CMsg::Max_50Mbps,
		CMsg::High_10Mbps,
		CMsg::Normal_1Mbps,
		CMsg::Low_500Kbps,
		CMsg::Min_100Kbps,
		NULL };
	return list;
}
#endif

bool MMF_REC_AUDIO::Start(_TCHAR *path, size_t path_size, int sample_rate)
{
#if 0
	const int bit_rates[] = {
		 50000000,	// max
	 	 10000000,	// high
		  1000000,	// normal
		   500000,	// low
		   100000,	// min
		0
	};
#endif
	UINT32 bits_per_sample = 16;
	UINT32 channels = 2;
	UINT32 bit_rate = 192000;	// max192000

	int codTypeNum = emu->get_parami(VM::ParamRecAudioCodec);
//	int quoTypeNum = emu->get_parami(VM::ParamRecAudioQuality);

	IMFMediaType *otype = NULL;
	IMFMediaType *itype = NULL;

//	HEAACWAVEINFO aac_config_info;
//	WMAUDIO2WAVEFORMAT wma2_config_info;
//	WMAUDIO3WAVEFORMAT wma3_config_info;
	IMFAttributes *attr = NULL;

	HRESULT hr;

	MMF_ReInitializeOnEmuThread();

	if (sample_rate <= 0) {
		return false;
	}
	if (codTypeNum < 0 || CODEC_UNKNOWN <= codTypeNum) {
		return false;
	}
#if 0
	if (quoTypeNum < 0 || 5 <= quoTypeNum) {
		return false;
	}
#endif

	// format name
	UTILITY::tcscat(path, path_size, codTypes[codTypeNum].ext);
	rec_path = path;

	rec_rate = sample_rate;
	bytes_per_sample = channels * bits_per_sample / 8;
	
	sample_buffer_size = rec_rate / 10;	// 0.1s
	// 100ns unit
//	frame_duration = ((LONGLONG)1000 * sample_buffer_size / sample_rate) * 10 * 1000;
	frame_duration = 1000 * 1000;	// sample_buffer_size / sample_rate = 0.1

	logging->out_logf(LOG_DEBUG, _T("MMF_REC_AUDIO::Start: %d"), codTypeNum);

	// open file for output
	if (codTypes[codTypeNum].cnt.Data1) {
		hr = MMF_CreateAttributes(&attr, 1);
		if (FAILED(hr)) {
			MMF_OutLog(LOG_ERROR, _T("MMF_CreateAttributes Failed."), hr);
			goto FIN;
		}
		attr->SetGUID(MF_TRANSCODE_CONTAINERTYPE, codTypes[codTypeNum].cnt);
	}

	// create sink writer
	hr = MMF_CreateSinkWriterFromURL(rec_path, attr, &sinkWriter);
	if (FAILED(hr)) {
		MMF_OutLog(LOG_ERROR, _T("MMF_CreateSinkWriterFromURL Failed."), hr);
		goto FIN;
	}
	SafeRelease(attr);

	// create media type for output
//	hr = MMF_TranscodeGetAudioOutputType(codTypes[codTypeNum].cod, &otype);
//	if (FAILED(hr)) {
		hr = MMF_CreateMediaType(&otype);
		if (FAILED(hr)) {
			MMF_OutLog(LOG_ERROR, _T("MMF_CreateMediaType 1 Failed."), hr);
			goto FIN;
		}
		hr = otype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		hr = otype->SetGUID(MF_MT_SUBTYPE, codTypes[codTypeNum].cod);
//	}

	// MF_MT_AUDIO_NUM_CHANNELS            {UINT32}
	hr = otype->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
	switch(codTypes[codTypeNum].num) {
	case CODEC_WAVE:
		// Linear PCM
//		sample_is_multiple = 0;
		// MF_MT_AUDIO_SAMPLES_PER_SECOND      {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rec_rate);
		// MF_MT_AUDIO_BITS_PER_SAMPLE         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits_per_sample);
		// MF_MT_AUDIO_BLOCK_ALIGNMENT         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, bytes_per_sample); // channel * int16_t
		// MF_MT_AUDIO_AVG_BYTES_PER_SECOND    {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, rec_rate * bytes_per_sample);
		// MF_MT_ALL_SAMPLES_INDEPENDENT   {UINT32 (BOOL)}
		hr = otype->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
		break;

	case CODEC_AAC:
	case CODEC_AAC4:
	case CODEC_ADTS:
		// AAC
//		sample_is_multiple = 1;
		// bit rate
		// MF_MT_AUDIO_AVG_BYTES_PER_SECOND    {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bit_rate / 8);
		// MF_MT_AUDIO_BITS_PER_SAMPLE         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits_per_sample);
//		// MF_MT_AUDIO_CHANNEL_MASK            {UINT32}
//		hr = otype->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT);
		// MF_MT_AUDIO_SAMPLES_PER_SECOND      {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rec_rate);
		// MF_MT_ALL_SAMPLES_INDEPENDENT   {UINT32 (BOOL)}
		hr = otype->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, FALSE);
		// MF_MT_AUDIO_BLOCK_ALIGNMENT         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
		// MF_MT_AAC_PAYLOAD_TYPE              {UINT32}
		hr = otype->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, (codTypes[codTypeNum].num == CODEC_ADTS ? 1 : 0));
		// MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION   {UINT32}
		hr = otype->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, (codTypes[codTypeNum].num == CODEC_AAC4 ? 0x2a : 0x29));
		//
#if 0
		memset(&aac_config_info, 0, sizeof(aac_config_info));
		aac_config_info.wfx.cbSize = sizeof(aac_config_info) - sizeof(WAVEFORMATEX);
		aac_config_info.wfx.wFormatTag = WAVE_FORMAT_MPEG_HEAAC;
		aac_config_info.wfx.nChannels = channels;
		aac_config_info.wfx.nSamplesPerSec = rec_rate;
		aac_config_info.wfx.nAvgBytesPerSec =  bit_rate / 8;
		aac_config_info.wfx.nBlockAlign = 1;
		aac_config_info.wPayloadType = (codTypes[codTypeNum].num == CODEC_ADTS ? 1 : 0);
		aac_config_info.wAudioProfileLevelIndication = 0x29;
		aac_config_info.wStructType = 0;
		// MF_MT_USER_DATA                    {BLOB} 
		hr = otype->SetBlob(MF_MT_USER_DATA, (UINT8 *)&aac_config_info, (UINT32)sizeof(aac_config_info));
#endif
		break;

#ifdef USE_MMF_AUDIO_WMA
	case CODEC_WMAV2:
		// Windows Media Audio
//		sample_is_multiple = 1;
		// bit rate
		// MF_MT_AUDIO_AVG_BYTES_PER_SECOND    {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 8005); //bit_rate / 8);
		// MF_MT_AUDIO_BITS_PER_SAMPLE         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits_per_sample);
//		// MF_MT_AUDIO_CHANNEL_MASK            {UINT32}
//		hr = otype->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT);
		// MF_MT_AUDIO_SAMPLES_PER_SECOND      {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rec_rate);
		// MF_MT_ALL_SAMPLES_INDEPENDENT   {UINT32 (BOOL)}
		hr = otype->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, FALSE);
		// MF_MT_AUDIO_BLOCK_ALIGNMENT         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1487); //1200);
#if 0
		//
		memset(&wma2_config_info, 0, sizeof(wma2_config_info));
		wma2_config_info.wfx.cbSize = sizeof(wma2_config_info) - sizeof(WAVEFORMATEX);
		wma2_config_info.wfx.wFormatTag = WAVE_FORMAT_WMAUDIO2;
		wma2_config_info.wfx.nChannels = channels;
		wma2_config_info.wfx.nSamplesPerSec = rec_rate;
		wma2_config_info.wfx.nAvgBytesPerSec =  8005;
		wma2_config_info.wfx.nBlockAlign = 1487;
		wma2_config_info.wfx.wBitsPerSample = bits_per_sample;
		wma2_config_info.dwSamplesPerBlock = 1487;
		wma2_config_info.wEncodeOptions = 0xe0;
		wma2_config_info.dwSuperBlockAlign = wma2_config_info.wfx.nBlockAlign;

		// MF_MT_USER_DATA                    {BLOB} 
		hr = otype->SetBlob(MF_MT_USER_DATA, (UINT8 *)&wma2_config_info, (UINT32)sizeof(wma2_config_info));

#endif
		break;

	case CODEC_WMAV3:
		// Windows Media Audio
//		sample_is_multiple = 1;
		// bit rate
		// MF_MT_AUDIO_AVG_BYTES_PER_SECOND    {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 8005);
		// MF_MT_AUDIO_BITS_PER_SAMPLE         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits_per_sample);
		// MF_MT_AUDIO_CHANNEL_MASK            {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT);
		// MF_MT_AUDIO_SAMPLES_PER_SECOND      {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rec_rate);
		// MF_MT_ALL_SAMPLES_INDEPENDENT   {UINT32 (BOOL)}
		hr = otype->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, FALSE);
		// MF_MT_AUDIO_BLOCK_ALIGNMENT         {UINT32}
		hr = otype->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1487);
#if 0
		//
		// WMA3 "6201 0200 44AC0000 451F0000 CF05 1000  1200  1000 03000000 00000000 00000000 E000 42C0"
		memset(&wma3_config_info, 0, sizeof(wma3_config_info));
		wma3_config_info.wfx.cbSize = sizeof(wma3_config_info) - sizeof(WAVEFORMATEX);
		wma3_config_info.wfx.wFormatTag = WAVE_FORMAT_WMAUDIO3;
		wma3_config_info.wfx.nChannels = channels;
		wma3_config_info.wfx.nSamplesPerSec = rec_rate;
		wma3_config_info.wfx.nAvgBytesPerSec =  8005;
		wma3_config_info.wfx.nBlockAlign = 1487;
		wma3_config_info.wfx.wBitsPerSample = bits_per_sample;
		wma3_config_info.wValidBitsPerSample = bits_per_sample;
		wma3_config_info.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		wma3_config_info.wEncodeOptions = 0xe0;

		// MF_MT_USER_DATA                    {BLOB} 
		hr = otype->SetBlob(MF_MT_USER_DATA, (UINT8 *)&wma3_config_info, (UINT32)sizeof(wma3_config_info));
#endif
		break;
#endif /* USE_MMF_AUDIO_WMA */
	}

	hr = sinkWriter->AddStream(otype, &streamIndex);
	if (FAILED(hr)) {
		MMF_OutLog(LOG_ERROR, _T("IMFSinkWriter::AddStream Failed."), hr);
		goto FIN;
	}

	// create media type for input
	hr = MMF_CreateMediaType(&itype);
	if (FAILED(hr)) {
		MMF_OutLog(LOG_ERROR, _T("MMF_CreateMediaType 2 Failed."), hr);
		goto FIN;
	}
	itype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	itype->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	// MF_MT_AUDIO_NUM_CHANNELS            {UINT32}
	itype->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
	// MF_MT_AUDIO_BITS_PER_SAMPLE         {UINT32}
	itype->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bits_per_sample);
	// MF_MT_AUDIO_SAMPLES_PER_SECOND      {UINT32}
	itype->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rec_rate);
	// MF_MT_ALL_SAMPLES_INDEPENDENT   {UINT32 (BOOL)}
	itype->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	// MF_MT_AUDIO_AVG_BYTES_PER_SECOND    {UINT32}
	itype->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, rec_rate * bytes_per_sample);
	// MF_MT_AUDIO_BLOCK_ALIGNMENT         {UINT32}
	itype->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, bytes_per_sample); // channels * int16_t 
	hr = sinkWriter->SetInputMediaType(streamIndex, itype, NULL); 
	if (FAILED(hr)) {
		MMF_OutLog(LOG_ERROR, _T("IMFSinkWriter::SetInputMediaType Failed."), hr);
		goto FIN;
	}

	// create sample buffer
	for(int n=0; n<BUFFER_COUNT; n++) {
		hr = MMF_CreateMemoryBuffer(sample_buffer_size * bytes_per_sample, &mediaBuffers[n]);
		if (FAILED(hr)) {
			MMF_OutLog(LOG_ERROR, _T("MMF_CreateMemoryBuffer Failed."), hr);
			goto FIN;
		}
		// Create a media sample and add the buffer to the sample.
		hr = MMF_CreateSample(&mediaSamples[n]);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: MMF_CreateSample Failed."), hr); 
			goto FIN;
		}

		hr = mediaSamples[n]->AddBuffer(mediaBuffers[n]);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSample::AddBuffer Failed."), hr); 
			goto FIN;
		}
	}

	buffer_idx = 0;

	// Tell the sink writer to start accepting data.
	hr = sinkWriter->BeginWriting();
	if (FAILED(hr)) {
		MMF_OutLog(LOG_ERROR, _T("IMFSinkWriter::BeginWriting Failed."), hr);
		goto FIN;
	}

    SafeRelease(otype);
    SafeRelease(itype);

	sample_time = 0;
	store_sample_pos = 0;
	write_error_count = 0;

	return true;

FIN:
    SafeRelease(otype);
    SafeRelease(itype);

	Release();
	RemoveFile();
	logging->out_log_x(LOG_ERROR, CMsg::Couldn_t_start_recording_audio);
	return false;
}

void MMF_REC_AUDIO::Stop()
{
	HRESULT hr;

	hr = WriteSample();
	hr = sinkWriter->Finalize();
	if (FAILED(hr)) {
		MMF_OutLog(LOG_ERROR, _T("Stop: IMFSinkWriter::Finalize Failed."), hr); 
	}
	Release();
}

bool MMF_REC_AUDIO::Restart()
{
	audio->Stop();
	return audio->Start(RECORD_AUDIO_TYPE_MMF, rec_rate, false);
}

bool MMF_REC_AUDIO::Record(int32_t *buffer, int samples)
{
	bool rc = false;
	BYTE *p;
	int16_t *store_samples;
	HRESULT hr;

	hr = mediaBuffers[buffer_idx]->Lock(&p, NULL, NULL);
	if (FAILED(hr)) {
		if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("Record: IMFMediaBuffer::Lock Failed."), hr); 
		goto FIN;
	}
	store_samples = (int16_t *)p;

	store_samples += (store_sample_pos << 1);

	for(int i=0; i<(samples << 1); i+=2) {
#ifdef USE_AUDIO_U8
		store_samples[0] = (int16_t)((buffer[i] - 128) * 256);
		store_samples[1] = (int16_t)((buffer[i+1] - 128) * 256);
#else
		store_samples[0] = (int16_t)buffer[i];
		store_samples[1] = (int16_t)buffer[i+1];
#endif
		store_samples += 2;
		store_sample_pos++;

		// Set the data length of the buffer.
		hr = mediaBuffers[buffer_idx]->SetCurrentLength(store_sample_pos * bytes_per_sample);

		if (store_sample_pos >= sample_buffer_size) {
			mediaBuffers[buffer_idx]->Unlock();

			hr = WriteSample();
			if (FAILED(hr)) {
				break;
			}

			// continue store data
			store_sample_pos = 0;

			hr = mediaBuffers[buffer_idx]->Lock(&p, NULL, NULL);
			store_samples = (int16_t *)p;
		}
	}
	mediaBuffers[buffer_idx]->Unlock();

FIN:
	rc = (SUCCEEDED(hr));
	if (!rc) {
		write_error_count++;
	}
	return rc;
}

HRESULT MMF_REC_AUDIO::WriteSample()
{
	HRESULT hr;

	do {
		// Set the time stamp and the duration.
		hr = mediaSamples[buffer_idx]->SetSampleTime(sample_time);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSample::SetSampleTime Failed."), hr); 
			break;
		}
		sample_time += frame_duration;

		hr = mediaSamples[buffer_idx]->SetSampleDuration(frame_duration);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSample::SetSampleDuration Failed."), hr); 
			break;
		}

#if 0
		MF_SINK_WRITER_STATISTICS streamStat;
		do {
			memset(&streamStat, 0, sizeof(streamStat));
			streamStat.cb = sizeof(streamStat);
			hr = sinkWriter->GetStatistics(streamIndex, &streamStat);
		} while (streamStat.dwByteCountQueued > 0);
#endif

		hr = sinkWriter->WriteSample(streamIndex, mediaSamples[buffer_idx]);

		// replace target of buffer
		buffer_idx = (buffer_idx + 1) % BUFFER_COUNT;

		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSinkWriter::WriteSample Failed."), hr); 
			break;
		}
	} while(0);

	return hr;
}

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_MMF */
