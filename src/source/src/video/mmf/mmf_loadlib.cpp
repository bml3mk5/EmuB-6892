/** @file mmf_loadlib.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ load media foundation library ]

	@note This component can use on Windows7 or later.
*/

#include "mmf_loadlib.h"

#if defined(USE_REC_VIDEO_MMF) || defined(USE_REC_AUDIO_MMF)

#pragma comment(lib,"mfuuid.lib")

#include "../../emu.h"

extern EMU *emu;

#ifndef USE_MMF_EXTERNAL_DLL

#pragma comment(lib,"mf.lib")
#pragma comment(lib,"mfplat.lib")
//#pragma comment(lib,"mfreadwrite.lib")
//#pragma comment(lib,"evr.lib")

#else /* USE_MMF_EXTERNAL_DLL */

#include "../../loadlibrary.h"
#include "../../utility.h"

static HINSTANCE hMMFLoader = NULL;

//
// entry point of mmf_loader.dll
//
HRESULT (*F_MMF_Startup)() = NULL;
HRESULT (*F_MMF_Shutdown)() = NULL;

HRESULT (*F_MMF_CreateAttributes)( IMFAttributes **ppMFAttributes, UINT32 cInitialSize ) = NULL;
HRESULT (*F_MMF_CreateMediaType)( IMFMediaType** ppMFType ) = NULL;
HRESULT (*F_MMF_CreateSample)( IMFSample **ppIMFSample ) = NULL;
HRESULT (*F_MMF_CopyImage)( BYTE* pDest,
	LONG        lDestStride,
	const BYTE* pSrc,
	LONG        lSrcStride,
	DWORD       dwWidthInBytes,
	DWORD       dwLines
) = NULL;
HRESULT (*F_MMF_CreateMemoryBuffer)( DWORD cbMaxLength, IMFMediaBuffer **ppBuffer ) = NULL;

HRESULT (*F_MMF_CreateSinkWriterFromURL)( const _TCHAR *pURL, IMFAttributes *pAttributes, IMFSinkWriter **ppSinkWriter ) = NULL;

//HRESULT (*F_MMF_TranscodeGetAudioOutputType)( REFGUID guidSubType, IMFMediaType** ppType ) = NULL;


#endif /* USE_MMF_EXTERNAL_DLL */

static bool loaded = false;
static int reffer = 0;

static bool loaded_emuthread = false;

bool MMF_LoadLibrary(int reffer_num)
{
	int i = 0;
	HRESULT hr;

	try {
#ifndef USE_MMF_EXTERNAL_DLL

	if (!loaded || reffer == 0) {

		hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
		loaded = (SUCCEEDED(hr));
	}

#else /* USE_MMF_EXTERNAL_DLL */

	while(!loaded && i == 0) {
		i++;

		// can load mmf_loader.dll
		LOAD_LIB(hMMFLoader, "mmf_loader", 0);

		// mmf_loader.dll
		GET_ADDR(F_MMF_Startup, HRESULT (*)(), hMMFLoader, "MMF_Startup");
		GET_ADDR(F_MMF_Shutdown, HRESULT (*)(), hMMFLoader, "MMF_Shutdown");
		GET_ADDR(F_MMF_CreateAttributes, HRESULT (*)(IMFAttributes **, UINT32), hMMFLoader, "MMF_CreateAttributes");
		GET_ADDR(F_MMF_CreateMediaType, HRESULT (*)(IMFMediaType**), hMMFLoader, "MMF_CreateMediaType");
		GET_ADDR(F_MMF_CreateSample, HRESULT (*)(IMFSample **), hMMFLoader, "MMF_CreateSample");
		GET_ADDR(F_MMF_CopyImage, HRESULT (*)(BYTE*, LONG, const BYTE*, LONG, DWORD, DWORD), hMMFLoader, "MMF_CopyImage");
		GET_ADDR(F_MMF_CreateMemoryBuffer, HRESULT (*)(DWORD, IMFMediaBuffer **), hMMFLoader, "MMF_CreateMemoryBuffer");
		GET_ADDR(F_MMF_CreateSinkWriterFromURL, HRESULT (*)(const _TCHAR *, IMFAttributes *, IMFSinkWriter **), hMMFLoader, "MMF_CreateSinkWriterFromURL");
//		GET_ADDR(F_MMF_TranscodeGetAudioOutputType, HRESULT (*)(REFGUID, IMFMediaType**), hMMFLoader, "MMF_TranscodeGetAudioOutputType");

		hr = F_MMF_Startup();
		if (FAILED(hr)) {
			break;
		}

		loaded = true;
	}

#endif /* USE_MMF_EXTERNAL_DLL */
	} catch(...) {
		// nothing to do
	}

	if (loaded) {
		reffer |= reffer_num;
		return true;
	} else {
		return false;
	}
}

void MMF_UnloadLibrary(int reffer_num)
{
	reffer &= ~reffer_num;
	if (reffer) return;

#ifndef USE_MMF_EXTERNAL_DLL
	if (loaded) {
		MFShutdown();
	}
#else /* USE_MMF_EXTERNAL_DLL */
	if (loaded) {
		F_MMF_Shutdown();
	}


	UNLOAD_LIB(hMMFLoader);
#endif /* USE_MMF_EXTERNAL_DLL */

	loaded = false;
}

void MMF_ReInitializeOnEmuThread()
{
	if (loaded && !loaded_emuthread) {
#ifndef USE_MMF_EXTERNAL_DLL

#else

#endif /* USE_MMF_EXTERNAL_DLL */
		loaded_emuthread = true;
	}
}

/*
	wrapper functions

 */
HRESULT MMF_CreateAttributes( IMFAttributes **ppMFAttributes, UINT32 cInitialSize )
{
	HRESULT hr;
#ifndef USE_MMF_EXTERNAL_DLL
	hr = MFCreateAttributes(
#else
	hr = F_MMF_CreateAttributes(
#endif /* USE_MMF_EXTERNAL_DLL */
		ppMFAttributes, cInitialSize
	);
	return hr;
}

HRESULT MMF_CreateMediaType( IMFMediaType** ppMFType )
{
	HRESULT hr;
#ifndef USE_MMF_EXTERNAL_DLL
	hr = MFCreateMediaType(
#else
	hr = F_MMF_CreateMediaType(
#endif /* USE_MMF_EXTERNAL_DLL */
		ppMFType
	);
	return hr;
}

HRESULT MMF_CreateSample( IMFSample **ppIMFSample )
{
	HRESULT hr;
#ifndef USE_MMF_EXTERNAL_DLL
	hr = MFCreateSample(
#else
	hr = F_MMF_CreateSample(
#endif /* USE_MMF_EXTERNAL_DLL */
		ppIMFSample
	);
	return hr;
}

HRESULT MMF_CopyImage( BYTE* pDest,
	LONG        lDestStride,
	const BYTE* pSrc,
	LONG        lSrcStride,
	DWORD       dwWidthInBytes,
	DWORD       dwLines
)
{
	HRESULT hr;
#ifndef USE_MMF_EXTERNAL_DLL
	hr = MFCopyImage(
#else
	hr = F_MMF_CopyImage(
#endif /* USE_MMF_EXTERNAL_DLL */
		pDest,
		lDestStride,
		pSrc,
		lSrcStride,
		dwWidthInBytes,
		dwLines
	);
	return hr;
}

HRESULT MMF_CreateMemoryBuffer( DWORD cbMaxLength, IMFMediaBuffer **ppBuffer )
{
	HRESULT hr;
#ifndef USE_MMF_EXTERNAL_DLL
	hr = MFCreateMemoryBuffer(
#else
	hr = F_MMF_CreateMemoryBuffer(
#endif /* USE_MMF_EXTERNAL_DLL */
		 cbMaxLength, ppBuffer );
	return hr;
}

HRESULT MMF_CreateSinkWriterFromURL( const _TCHAR *pURL, IMFAttributes *pAttributes, IMFSinkWriter **ppSinkWriter )
{
	HRESULT hr;
#ifndef USE_MMF_EXTERNAL_DLL
	IMFReadWriteClassFactory *rwf = NULL;
	WCHAR wURL[_MAX_PATH];

	UTILITY::mbs_to_wcs(wURL, pURL);
	hr = CoCreateInstance(CLSID_MFReadWriteClassFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&rwf));
	if (SUCCEEDED(hr)) {
		hr = rwf->CreateInstanceFromURL(CLSID_MFSinkWriter, wURL, pAttributes, IID_IMFSinkWriter, (LPVOID *)ppSinkWriter);
		rwf->Release();
	}
#else
	hr = F_MMF_CreateSinkWriterFromURL(pURL, pAttributes, ppSinkWriter);
#endif /* USE_MMF_EXTERNAL_DLL */
	return hr;
}

#if 0
HRESULT MMF_TranscodeGetAudioOutputType( REFGUID guidSubType, IMFMediaType** ppType )
{
	HRESULT hr;
	IMFCollection   *pAvailableTypes = NULL;
	IMFMediaType    *pAudioType = NULL;
	IUnknown *pUnk = NULL;

	DWORD dwFlags = (MFT_ENUM_FLAG_ALL & (~MFT_ENUM_FLAG_FIELDOFUSE)) | MFT_ENUM_FLAG_SORTANDFILTER;

	hr = MFTranscodeGetAudioOutputAvailableTypes(guidSubType, dwFlags, NULL, &pAvailableTypes);
	if (SUCCEEDED(hr)) {
	    hr = pAvailableTypes->GetElement(0, &pUnk);
	    if (SUCCEEDED(hr)) {
	        hr = pUnk->QueryInterface(IID_PPV_ARGS(&pAudioType));
			*ppType = pAudioType;
	        pUnk->Release();
		}
	}
	return hr;
}
#endif

void MMF_OutLog(int level, const _TCHAR *msg, HRESULT hr)
{
	_TCHAR msgf[_MAX_PATH];

	UTILITY::stprintf(msgf, _MAX_PATH, _T("%s [%x]"), msg, (int)hr);
	logging->out_log(level, msgf); 
}


#endif /* USE_REC_VIDEO_MMF || USE_REC_AUDIO_MMF */
