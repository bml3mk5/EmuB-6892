/** @file mmf_loader.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ load media foundation library ]

	@note This component can use on Windows7 or later.
*/

#include "mmf_loader.h"

EXTERN_C MMF_LOADER_API HRESULT MMF_Startup()
{
	HRESULT hr;
	hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_Shutdown()
{
	HRESULT hr;
	hr = MFShutdown();
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateAttributes( IMFAttributes **ppMFAttributes, UINT32 cInitialSize )
{
	HRESULT hr;
	hr = MFCreateAttributes( ppMFAttributes, cInitialSize );
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateMediaType( IMFMediaType** ppMFType )
{
	HRESULT hr;
	hr = MFCreateMediaType( ppMFType );
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateSample( IMFSample **ppIMFSample )
{
	HRESULT hr;
	hr = MFCreateSample( ppIMFSample );
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_CopyImage( BYTE* pDest,
	LONG        lDestStride,
	const BYTE* pSrc,
	LONG        lSrcStride,
	DWORD       dwWidthInBytes,
	DWORD       dwLines
)
{
	HRESULT hr;
	hr = MFCopyImage(
		pDest,
		lDestStride,
		pSrc,
		lSrcStride,
		dwWidthInBytes,
		dwLines
	);
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateMemoryBuffer( DWORD cbMaxLength, IMFMediaBuffer **ppBuffer )
{
	HRESULT hr;
	hr = MFCreateMemoryBuffer( cbMaxLength, ppBuffer );
	return hr;
}

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateSinkWriterFromURL( const _TCHAR *pURL, IMFAttributes *pAttributes, IMFSinkWriter **ppSinkWriter )
{
	HRESULT hr;
	IMFReadWriteClassFactory *rwf = NULL;
	WCHAR wURL[_MAX_PATH];

	MultiByteToWideChar(CP_ACP, 0, pURL, -1, wURL, _MAX_PATH);
	hr = CoCreateInstance(CLSID_MFReadWriteClassFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&rwf));
	if (SUCCEEDED(hr)) {
		hr = rwf->CreateInstanceFromURL(CLSID_MFSinkWriter, wURL, pAttributes, IID_IMFSinkWriter, (LPVOID *)ppSinkWriter);
		rwf->Release();
	}
	return hr;
}

#if 0
EXTERN_C MMF_LOADER_API HRESULT MMF_TranscodeGetAudioOutputType( REFGUID guidSubType, IMFMediaType** ppType )
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
