/** @file mmf_loadlib.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.10 -

	@brief [ load media foundation library ]

	@note This component can use on Windows7 or later.
*/

#ifndef _MMF_LOAD_LIBRARY_H_
#define _MMF_LOAD_LIBRARY_H_

#include "../../rec_video_defs.h"
#include "../../common.h"

#if defined(USE_REC_VIDEO_MMF) || defined(USE_REC_AUDIO_MMF)

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <evr.h>

/// Load dll when start recording video.
#define USE_MMF_EXTERNAL_DLL


bool MMF_LoadLibrary(int reffer_num);
void MMF_UnloadLibrary(int reffer_num);
void MMF_ReInitializeOnEmuThread();

HRESULT MMF_CreateAttributes( IMFAttributes **ppMFAttributes, UINT32 cInitialSize );
HRESULT MMF_CreateMediaType( IMFMediaType** ppMFType );
HRESULT MMF_CreateSample( IMFSample **ppIMFSample );
HRESULT MMF_CopyImage( BYTE* pDest,
	LONG        lDestStride,
	const BYTE* pSrc,
	LONG        lSrcStride,
	DWORD       dwWidthInBytes,
	DWORD       dwLines
);
HRESULT MMF_CreateMemoryBuffer( DWORD cbMaxLength, IMFMediaBuffer **ppBuffer );

HRESULT MMF_CreateSinkWriterFromURL( const _TCHAR *pURL, IMFAttributes *pAttributes, IMFSinkWriter **ppSinkWriter );

//HRESULT MMF_TranscodeGetAudioOutputType( REFGUID guidSubType, IMFMediaType** ppType );


void MMF_OutLog(int level, const _TCHAR *msg, HRESULT hr);

#endif /* USE_REC_VIDEO_MMF || USE_REC_AUDIO_MMF */

#endif /* _MMF_LOAD_LIBRARY_H_ */
