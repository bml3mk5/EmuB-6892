/** @file mmf_rec_video.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ record video using microsoft media foundation ]

	@note This component can use on Windows7 or later.
*/

#include "mmf_rec_video.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_MMF)

#include "../rec_video.h"
#include "../../emu.h"
#include "../../csurface.h"
#include "../../utility.h"

MMF_REC_VIDEO::MMF_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;
	rec_fps = 0;
	memset(&rec_rect, 0, sizeof(rec_rect));
	rec_surface = NULL;
	rec_path = NULL;

	streamIndex = 0;
	sample_time = 0;
	frame_duration = 0;
	stride = 1;
	write_error_count = 0;

	sinkWriter = NULL;
}

MMF_REC_VIDEO::~MMF_REC_VIDEO()
{
	Release();
	MMF_UnloadLibrary(USE_REC_VIDEO_MMF);
}

#define SafeRelease(object) { if (object) object->Release(); object = NULL; }

void MMF_REC_VIDEO::Release()
{
	SafeRelease(sinkWriter);
}

void MMF_REC_VIDEO::RemoveFile()
{
#if defined(_WIN32)
	DeleteFile(rec_path);
#else
	unlink(rec_path);
#endif
}

bool MMF_REC_VIDEO::IsEnabled()
{
	return MMF_LoadLibrary(USE_REC_VIDEO_MMF);
}

enum codec_ids {
	CODEC_H264 = 0,
	CODEC_MP4V,
	CODEC_WMV1,
	CODEC_WMV2,
	CODEC_UNKNOWN
};

typedef struct {
	enum codec_ids num;
	const GUID     cod;
	const _TCHAR  *ext;
	const int      stride;	// draw direction top->bottom:1 bottom->top:-1
} codTypes_t;

static const codTypes_t codTypes[] = {
	{ CODEC_H264, MFVideoFormat_H264, _T(".m4v"), 1 },
	{ CODEC_MP4V, MFVideoFormat_MP4V, _T(".m4v"), 1 },
	{ CODEC_WMV1, MFVideoFormat_WMV1, _T(".wmv"), -1 },
	{ CODEC_WMV2, MFVideoFormat_WMV2, _T(".wmv"), -1 },
};

const _TCHAR **MMF_REC_VIDEO::GetCodecList()
{
	static const _TCHAR *list[] = {
		_T("MPEG-4(H.264)"),
		_T("MPEG-4"),
		_T("WMV1"),
		_T("WMV2"),
		NULL };
	return list;
}

const CMsg::Id *MMF_REC_VIDEO::GetQualityList()
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

bool MMF_REC_VIDEO::Start(_TCHAR *path, size_t path_size, int fps, const VmRectWH *srcrect, CSurface *srcsurface, bool show_dialog)
{
	const int bit_rates[] = {
		 50000000,	// max
	 	 10000000,	// high
		  1000000,	// normal
		   500000,	// low
		   100000,	// min
		0
	};

	int codTypeNum = emu->get_parami(VM::ParamRecVideoCodec);
	int quoTypeNum = emu->get_parami(VM::ParamRecVideoQuality);

//	MFVIDEOFORMAT oformat;
	IMFMediaType *otype = NULL;
	IMFMediaType *itype = NULL;

	HRESULT hr;

	MMF_ReInitializeOnEmuThread();

	if (fps <= 0 || srcrect->w <= 0 || srcrect->h <= 0 || srcsurface == NULL) {
		return false;
	}
	if (codTypeNum < 0 || CODEC_UNKNOWN <= codTypeNum) {
		return false;
	}
	if (quoTypeNum < 0 || 5 <= quoTypeNum) {
		return false;
	}

	// format name
	UTILITY::tcscat(path, path_size, codTypes[codTypeNum].ext);
	rec_path = path;

	rec_fps = fps;
	rec_rect = *srcrect;
	rec_surface = srcsurface;

	stride = codTypes[codTypeNum].stride;

	frame_duration = 10 * 1000 * 1000 / rec_fps;

	logging->out_logf(LOG_DEBUG, _T("MMF_REC_VIDEO::Start: %d"), codTypeNum);

	// open file for output

	// create sink writer
	hr = MMF_CreateSinkWriterFromURL(rec_path, NULL, &sinkWriter);
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("MMF_CreateSinkWriterFromURL Failed: %d"), hr);
		goto FIN;
	}

	// create media type for output
#if 0
	memset(&oformat, 0, sizeof(oformat));
	oformat.dwSize = sizeof(oformat);
	oformat.videoInfo.dwWidth = rec_rect.w;
	oformat.videoInfo.dwHeight = rec_rect.h;
	oformat.videoInfo.PixelAspectRatio.Numerator = 1;
	oformat.videoInfo.PixelAspectRatio.Denominator = 1;
	oformat.videoInfo.InterlaceMode = MFVideoInterlace_Progressive;
	oformat.videoInfo.FramesPerSecond.Numerator = rec_fps;
	oformat.videoInfo.FramesPerSecond.Denominator = 1;

	oformat.guidFormat = codTypes[codTypeNum].cod;

	oformat.compressedInfo.AvgBitrate = bit_rates[quoTypeNum];
#endif

	hr = MMF_CreateMediaType(&otype);
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("MMF_CreateMediaType 1 Failed: %d"), hr);
		goto FIN;
	}
#if 1
	hr = otype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = otype->SetGUID(MF_MT_SUBTYPE, codTypes[codTypeNum].cod);
	// MF_MT_AVG_BITRATE               {UINT32}
	hr = otype->SetUINT32(MF_MT_AVG_BITRATE, bit_rates[quoTypeNum]);
	// MF_MT_INTERLACE_MODE            {UINT32 (oneof MFVideoInterlaceMode)}
	hr = otype->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);  
	// MF_MT_FRAME_SIZE                {UINT64 (HI32(Width),LO32(Height))}
	hr = otype->SetUINT64(MF_MT_FRAME_SIZE, PackSize(rec_rect.w, rec_rect.h));
	// MF_MT_FRAME_RATE                {UINT64 (HI32(Numerator),LO32(Denominator))}
	hr = otype->SetUINT64(MF_MT_FRAME_RATE, PackRatio(rec_fps, 1));
	// MF_MT_PIXEL_ASPECT_RATIO        {UINT64 (HI32(Numerator),LO32(Denominator))}
	hr = otype->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, PackRatio(1, 1));
#endif
	hr = sinkWriter->AddStream(otype, &streamIndex);
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("IMFSinkWriter::AddStream Failed: %d"), hr);
		goto FIN;
	}

	// create media type for input
	hr = MMF_CreateMediaType(&itype);
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("MMF_CreateMediaType 2 Failed: %d"), hr);
		goto FIN;
	}
	itype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	itype->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	// MF_MT_INTERLACE_MODE            {UINT32 (oneof MFVideoInterlaceMode)}
	itype->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);  
	// MF_MT_FRAME_SIZE                {UINT64 (HI32(Width),LO32(Height))}
	itype->SetUINT64(MF_MT_FRAME_SIZE, PackSize(rec_rect.w, rec_rect.h));
	// MF_MT_FRAME_RATE                {UINT64 (HI32(Numerator),LO32(Denominator))}
	itype->SetUINT64(MF_MT_FRAME_RATE, PackRatio(rec_fps, 1));
	// MF_MT_PIXEL_ASPECT_RATIO        {UINT64 (HI32(Numerator),LO32(Denominator))}
	itype->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, PackRatio(1, 1));

	hr = sinkWriter->SetInputMediaType(streamIndex, itype, NULL); 
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("IMFSinkWriter::SetInputMediaType Failed: %d"), hr);
		goto FIN;
	}

	// Tell the sink writer to start accepting data.
	hr = sinkWriter->BeginWriting();
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("IMFSinkWriter::BeginWriting Failed: %d"), hr);
		goto FIN;
	}

    SafeRelease(otype);
    SafeRelease(itype);

	sample_time = 0;
	write_error_count = 0;

	return true;

FIN:
    SafeRelease(otype);
    SafeRelease(itype);

	Release();
	RemoveFile();
	logging->out_log_x(LOG_ERROR, CMsg::Couldn_t_start_recording_video);
	return false;
}

void MMF_REC_VIDEO::Stop()
{
	HRESULT hr = sinkWriter->Finalize();
	if (FAILED(hr)) {
		logging->out_logf(LOG_ERROR, _T("Stop: IMFSinkWriter::Finalize Failed: %d"), hr); 
	}
	Release();
}

bool MMF_REC_VIDEO::Restart()
{
	vid->Stop();
	return vid->Start(RECORD_VIDEO_TYPE_MMF, -1, vid->GetSrcRect(), NULL, false);
}

bool MMF_REC_VIDEO::Record()
{
	bool rc = false;

	IMFMediaBuffer *pBuffer = NULL;

	const LONG  cbWidth = 4 * rec_rect.w;
	const DWORD cbBuffer = cbWidth * rec_rect.h;

	BYTE *pData = NULL;

	HRESULT hr;

	// Create a new memory buffer.
	hr = MMF_CreateMemoryBuffer(cbBuffer, &pBuffer);
	if (FAILED(hr)) {
		if (!write_error_count) logging->out_logf(LOG_ERROR, _T("Record: MMF_CreateMemoryBuffer Failed: %d"), hr); 
		goto FIN;
	}
	hr = pBuffer->Lock(&pData, NULL, NULL);
	if (FAILED(hr)) {
		if (!write_error_count) logging->out_logf(LOG_ERROR, _T("Record: IMFMediaBuffer::Lock Failed: %d"), hr); 
		goto FIN;
	}
	if (stride < 0) {
		pData += (cbWidth * (rec_rect.h - 1)); 
	}
	hr = MMF_CopyImage(
		pData,                      // Destination buffer.
		cbWidth * stride,           // Destination stride.
		(BYTE *)rec_surface->GetBuffer(), // First row in source image.
		cbWidth,                    // Source stride.
		cbWidth,                    // Image width in bytes.
		rec_rect.h                  // Image height in pixels.
	);
	pBuffer->Unlock();

    // Set the data length of the buffer.
	hr = pBuffer->SetCurrentLength(cbBuffer);
    if (FAILED(hr)) {
		if (!write_error_count) logging->out_logf(LOG_ERROR, _T("Record: IMFMediaBuffer::SetCurrentLength Failed: %d"), hr); 
		goto FIN;
    }

	hr = WriteSample(pBuffer);
	if (FAILED(hr)) {
		goto FIN;
	}
FIN:
	rc = (SUCCEEDED(hr));
	if (!rc) {
		write_error_count++;
	}
    SafeRelease(pBuffer);
	return rc;
}

HRESULT MMF_REC_VIDEO::WriteSample(IMFMediaBuffer *pBuffer)
{
	IMFSample *pSample = NULL;
	HRESULT hr;

	do {
		// Create a media sample and add the buffer to the sample.
		hr = MMF_CreateSample(&pSample);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: MMF_CreateSample Failed."), hr); 
			break;
		}

		hr = pSample->AddBuffer(pBuffer);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSample::AddBuffer Failed."), hr); 
			break;
		}

		// Set the time stamp and the duration.
		hr = pSample->SetSampleTime(sample_time);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSample::SetSampleTime Failed."), hr); 
			break;
		}
		sample_time += frame_duration;

		hr = pSample->SetSampleDuration(frame_duration);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSample::SetSampleDuration Failed."), hr); 
			break;
		}

		hr = sinkWriter->WriteSample(streamIndex, pSample);
		if (FAILED(hr)) {
			if (!write_error_count) MMF_OutLog(LOG_ERROR, _T("WriteSample: IMFSinkWriter::WriteSample Failed."), hr); 
			break;
		}
	} while(0);

	SafeRelease(pSample);

	return hr;
}

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_MMF */
