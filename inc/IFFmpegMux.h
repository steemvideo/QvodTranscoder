#ifndef __H_ffmpegMuxGuids__
#define __H_ffmpegMuxGuids__
#include "TranscodeStruct.h"

// {044BAFB4-1908-4fb9-BFB8-75CEA950ECA0}
DEFINE_GUID(CLSID_QvodFFmpegMux, 
			0x44bafb4, 0x1908, 0x4fb9, 0xbf, 0xb8, 0x75, 0xce, 0xa9, 0x50, 0xec, 0xa0);

// {4722DF8D-C7AD-4a43-8F7F-7FB66066F0E8}
DEFINE_GUID(IID_IFFmpegMux, 
			0x4722df8d, 0xc7ad, 0x4a43, 0x8f, 0x7f, 0x7f, 0xb6, 0x60, 0x66, 0xf0, 0xe8);


#ifdef __cplusplus
extern "C" {
#endif

DECLARE_INTERFACE_(IFFmpegMux, IUnknown)
{
	STDMETHOD (SetMuxConfig) (MUX_CONFIG* pMuxConfig) = 0;
	STDMETHOD (GetMuxConfig) (MUX_CONFIG* pMuxConfig) = 0;
	STDMETHOD (SetMuxThreadCount) (DWORD Threadcount) = 0;
	STDMETHOD (GetMuxThreadCount) (DWORD &Threadcount) = 0;
	STDMETHOD (SetMuxDuration) (REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop) = 0;
	STDMETHOD (GetMuxDuration) (REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop) = 0;
	STDMETHOD (SetSrcFrameRate) (float framerate) = 0;
	STDMETHOD (SetQvodCallBack) (interface IQvodCallBack* pQvodCallBack) = 0;

};
	//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __H_ffmpegSourceGuids__

