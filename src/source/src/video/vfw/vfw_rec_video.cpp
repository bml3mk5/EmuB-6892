/** @file vfw_rec_video.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ record video using video for windows ]
*/

#include "../../rec_video_defs.h"

#if defined(USE_REC_VIDEO) && defined(USE_REC_VIDEO_VFW)

#if defined(USE_WX) || defined(USE_WX2)
#include <wx/wx.h>
#endif
#include "vfw_rec_video.h"
#include <windowsx.h>
#include <mmsystem.h>
#include "../rec_video.h"
#include "../../emu_osd.h"
#include "../../csurface.h"
#include "../../fileio.h"
#include "../../utility.h"
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL_syswm.h>
#endif
#if defined(USE_QT)
#include <QApplication>
#include <QWidget>
#endif

#ifdef _MSC_VER
#pragma comment(lib,"vfw32.lib")
#endif

VFW_REC_VIDEO::VFW_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;
	rec_fps = 0;
	memset(&rec_rect, 0, sizeof(rec_rect));
	rec_surface = NULL;
	rec_path = NULL;

	// vfw
	pAVIStream = NULL;
	pAVICompressed = NULL;
	pAVIFile = NULL;
	memset(&opts, 0, sizeof(opts));
}

VFW_REC_VIDEO::~VFW_REC_VIDEO()
{
	Release();
}

void VFW_REC_VIDEO::Release()
{
	if(pAVIStream) {
		AVIStreamClose(pAVIStream);
		pAVIStream = NULL;
	}
	if(pAVICompressed) {
		AVIStreamClose(pAVICompressed);
		pAVICompressed = NULL;
	}
	if(pAVIFile) {
		AVIFileClose(pAVIFile);
		AVIFileExit();
		pAVIFile = NULL;
	}
}

void VFW_REC_VIDEO::RemoveFile()
{
	FILEIO::RemoveFile(rec_path);
}

bool VFW_REC_VIDEO::IsEnabled()
{
	return true;
}


const _TCHAR **VFW_REC_VIDEO::GetCodecList()
{
	return NULL;
}

const _TCHAR **VFW_REC_VIDEO::GetQualityList()
{
	return NULL;
}

bool VFW_REC_VIDEO::Start(_TCHAR *path, size_t path_size, int fps, const VmRectWH *srcrect, CSurface *srcsurface, bool show_dialog)
{
	AVISTREAMINFO strhdr;
	AVICOMPRESSOPTIONS FAR * pOpts[1];

#if defined(USE_SDL) || defined(USE_SDL2)
	SDL_SysWMinfo info;
#endif
	HWND hWindow;
#if defined(_RGB565)
	LPDWORD lpBf;
#endif

	if (path != NULL) {
		UTILITY::tcscat(path, path_size, _T(".avi"));
		rec_path = path;
	}
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

	HRESULT res;

	// initialize vfw
	CTchar crec_path(rec_path);
	AVIFileInit();
	if((res = AVIFileOpen(&pAVIFile, crec_path.GetM(), OF_WRITE | OF_CREATE, NULL)) != AVIERR_OK) {
		logging->out_logf(LOG_ERROR,_T("AVIFileOpen failed to open. code=0x%x"), res);
		goto FIN;
	}

	// stream header
	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType = streamtypeVIDEO;	// vids
	strhdr.fccHandler = 0;
	strhdr.dwScale = 1;
	strhdr.dwRate = rec_fps;
	strhdr.dwSuggestedBufferSize = rec_surface->GetBufferSize();
	SetRect(&strhdr.rcFrame, 0, 0, rec_rect.w, rec_rect.h);
	if((res = AVIFileCreateStream(pAVIFile, &pAVIStream, &strhdr)) != AVIERR_OK) {
		logging->out_logf(LOG_ERROR,_T("AVIFileCreateStream: Failed. 0x%x"),res);
		goto FIN;
	}

#if defined(USE_WIN)
	hWindow = ((EMU_OSD *)emu)->get_window();
#elif defined(USE_SDL)
	SDL_VERSION(&info.version);
	SDL_GetWMInfo(&info);
	hWindow = info.window;
#elif defined(USE_SDL2)
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(((EMU_OSD *)emu)->get_window(), &info);
	hWindow = info.info.win.window;
#elif defined(USE_WX) || defined(USE_WX2)
	hWindow = (((EMU_OSD *)emu)->get_window())->GetHandle();
#elif defined(USE_QT)
	QWidget *win;
	win = QApplication::activeWindow();
	hWindow = (HWND)win->winId();
#endif

	// compression
	pOpts[0] = &opts;
	if(show_dialog && !(AVISaveOptions(hWindow, ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE, 1, &pAVIStream, (LPAVICOMPRESSOPTIONS FAR *)&pOpts))) {
		AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR *)&pOpts);
		goto FIN;
	}
	if((res = AVIMakeCompressedStream(&pAVICompressed, pAVIStream, &opts, NULL)) != AVIERR_OK) {
		logging->out_logf(LOG_ERROR,_T("AVIMakeCompressedStream: Failed. 0x%x"), res);
		goto FIN;
	}

#if defined(USE_WIN)
	if((res = AVIStreamSetFormat(pAVICompressed, 0, rec_surface->GetHeader(), rec_surface->GetHeaderSize())) != AVIERR_OK)
#else
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = rec_rect.w;
	bmi.bmiHeader.biHeight = rec_rect.h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = rec_surface->BitsPerPixel();
#if defined(_RGB555)
	bmi.bmiHeader.biCompression = BI_RGB;
#elif defined(_RGB565)
	bmi.bmiHeader.biCompression = BI_BITFIELDS;
	lpBf = (LPDWORD)*lpDib->bmiColors;
	lpBf[0] = 0x1f << 11;
	lpBf[1] = 0x3f << 5;
	lpBf[2] = 0x1f << 0;
#elif defined(_RGB888)
	bmi.bmiHeader.biCompression = BI_RGB;
#endif
	bmi.bmiHeader.biSizeImage = rec_surface->GetBufferSize();

	if((res = AVIStreamSetFormat(pAVICompressed, 0, &bmi.bmiHeader, bmi.bmiHeader.biSize)) != AVIERR_OK)
#endif
	{
		logging->out_logf(LOG_ERROR,_T("AVIStreamSetFormat: Failed. 0x%x"),res);
		goto FIN;
	}
	dwAVIFileSize = 0;
	lAVIFrames = 0;
	return true;

FIN:
	Release();
	RemoveFile();
	logging->out_log_x(LOG_ERROR, CMsg::Couldn_t_start_recording_video);
	return false;
}

void VFW_REC_VIDEO::Stop()
{
	// release vfw
	Release();

	// repair header
	CTchar crec_path(rec_path);
	FILE* fp = _tfopen(crec_path.GetM(), _T("r+b"));
	if(fp != NULL) {
		// copy fccHandler
		uint8_t buf[4];
		fseek(fp, 0xbc, SEEK_SET);
		if(ftell(fp) == 0xbc) {
			fread(buf, 4, 1, fp);
			fseek(fp, 0x70, SEEK_SET);
			fwrite(buf, 4, 1, fp);
		}
		fclose(fp);
	}
}

bool VFW_REC_VIDEO::Restart()
{
	vid->Stop();
	// create new file
	return vid->Start(RECORD_VIDEO_TYPE_VFW, -1, vid->GetSrcRect(), NULL, false);
}

bool VFW_REC_VIDEO::Record()
{
	bool rc = true;
	LONG lBytesWritten;
	rec_surface->Lock();
	if(AVIStreamWrite(pAVICompressed, lAVIFrames++, 1, (LPBYTE)rec_surface->GetBuffer(), rec_surface->GetBufferSize(), AVIIF_KEYFRAME, NULL, &lBytesWritten) == AVIERR_OK) {
		// if avi file size > (2GB - 16MB), create new avi file
		if((dwAVIFileSize += lBytesWritten) >= 0x7f000000L) {
			rc = Restart();
		}
	} else {
		logging->out_log(LOG_ERROR,_T("AVIStreamWrite: Failed."));
		vid->Stop();
		rc = false;
	}
	rec_surface->Unlock();
	return rc;
}

#endif /* USE_REC_VIDEO && USE_REC_VIDEO_VFW */
