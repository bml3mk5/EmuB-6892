/** @file mmf_loader.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ load media foundation library ]

	@note This component can use on Windows7 or later.
*/

#ifndef _MMF_LOADER_H_
#define _MMF_LOADER_H_

#include <Windows.h>
#include <tchar.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <evr.h>

#ifdef MMF_LOADER_EXPORTS
#define MMF_LOADER_API __declspec(dllexport)
#else
#define MMF_LOADER_API __declspec(dllimport)
#endif

#pragma comment(lib,"mfuuid.lib")
#pragma comment(lib,"mf.lib")
#pragma comment(lib,"mfplat.lib")
//#pragma comment(lib,"mfreadwrite.lib")
//#pragma comment(lib,"evr.lib")

EXTERN_C MMF_LOADER_API HRESULT MMF_Startup();
EXTERN_C MMF_LOADER_API HRESULT MMF_Shutdown();

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateAttributes( IMFAttributes **ppMFAttributes, UINT32 cInitialSize );
EXTERN_C MMF_LOADER_API HRESULT MMF_CreateMediaType( IMFMediaType** ppMFType );
EXTERN_C MMF_LOADER_API HRESULT MMF_CreateSample( IMFSample **ppIMFSample );
EXTERN_C MMF_LOADER_API HRESULT MMF_CopyImage( BYTE* pDest,
	LONG        lDestStride,
	const BYTE* pSrc,
	LONG        lSrcStride,
	DWORD       dwWidthInBytes,
	DWORD       dwLines
);
EXTERN_C MMF_LOADER_API HRESULT MMF_CreateMemoryBuffer( DWORD cbMaxLength, IMFMediaBuffer **ppBuffer );

EXTERN_C MMF_LOADER_API HRESULT MMF_CreateSinkWriterFromURL( const _TCHAR *pURL, IMFAttributes *pAttributes, IMFSinkWriter **ppSinkWriter );

//EXTERN_C MMF_LOADER_API HRESULT MMF_TranscodeGetAudioOutputType( REFGUID guidSubType, IMFMediaType** ppType );


#endif /* _MMF_LOADER_H_ */
