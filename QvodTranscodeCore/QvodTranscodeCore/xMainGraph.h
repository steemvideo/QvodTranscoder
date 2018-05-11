#pragma once
#include "IFFmpegMux.h"
#include "IQVODH265Encoder.h"
#include "IPushSource.h"
#include "IDataSource.h"
#include "xEdit.h"
#include "QVodFilterManager.h"
#ifdef _DEBUG
#define SHOW_GRAPH_IN_GRAPHEDIT
#endif
//debug info add by zwu
#define      OUTPUT_DETAIL_INFO          2
#define      OUTPUT_MAIN_INFO            1

#define      OUTPUT_WARNING_INFO         -1
#define      OUTPUT_ERROR_INFO           -2
#ifndef _DEBUG
#define ENABLE_LOG_LEVEL  2
#else
#define ENABLE_LOG_LEVEL   -2
#endif


typedef LONG    (WINAPI   *pfnDllGetClassObject)(REFCLSID , REFIID, LPVOID *);

#define BeginEnumPins(pBaseFilter, pEnumPins, pPin) \
{CComPtr<IEnumPins> pEnumPins; \
	if(pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins))) \
{ \
	for(CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
{ \

#define EndEnumPins }}}

#define BeginEnumMediaTypes(pPin, pEnumMediaTypes, pMediaType) \
{CComPtr<IEnumMediaTypes> pEnumMediaTypes; \
	if(pPin && SUCCEEDED(pPin->EnumMediaTypes(&pEnumMediaTypes))) \
{ \
	AM_MEDIA_TYPE* pMediaType = NULL; \
	for(; S_OK == pEnumMediaTypes->Next(1, &pMediaType, NULL); DeleteMediaType(pMediaType), pMediaType = NULL) \
{ \

#define EndEnumMediaTypes(pMediaType) } if(pMediaType) DeleteMediaType(pMediaType); }}

enum e_FilterType
{
	_EFT_SRC = 0,
	_EFT_SPL,
	_EFT_SUB,
	_EFT_A_DEC,
	_EFT_V_DEC,
	//_EFT_V_ENC,
	_EFT_A_TRANS,
	_EFT_V_TRANS,
	_EFT_AUDIO,
	_EFT_VIDEO,
	_EFT_SPY,
	_EFT_DEMUL,
	_EFT_MUX,
	_EFT_PSI,
	_EFT_WRIT
};

class xMainGraph
{
public:
	xMainGraph(boost::shared_ptr<IDataSource> datasource);
	xMainGraph();
	~xMainGraph(void);

	void	SetPlayWnd(HWND);
public:
	virtual CComPtr<IGraphBuilder>		BuildGraph();
	virtual void						DestroyGraph();
	
//	CComPtr<IFFmpegMux>					GetIFFmpegMux() { return mIFFmpegMux;}
	
	CComPtr<IPushSource>				GetIPushSource() { return mIPushSource;}
	static void Trace(int loglevel, const wchar_t* pstr, ...);


protected:	
	virtual	void	init();
	virtual bool	LoadVideoRender();
	virtual bool	LoadAudioRender();

	virtual	CComPtr<IBaseFilter>		LoadSouceFilter();


	CComPtr<IBaseFilter>		LoadFilter(const CLSID& clsid, QVOD_FILTER_INFO& filter_info);
	HMODULE						LoadModuleSpecy(CString path);
	CComPtr<IGraphBuilder>		LoadFilterGraph();

	CComPtr<IBaseFilter>		LoadMultiOutputFilter();
	bool						AddFiltertoGraph(CComPtr<IBaseFilter>& pbasefilter,  LPCWSTR pName);
	bool						ConnectFilers(CComPtr<IBaseFilter>& pSrc, CComPtr<IBaseFilter>& pDest);			// connect all output pin to in pin
	HRESULT						ConnectFilterDirect(CComPtr<IBaseFilter>& pSrc, CComPtr<IBaseFilter>& pDest);	// connect one free output pin to in pin
	HRESULT						ConnectPinToFilers(CComPtr<IPin>& pOutPin,CComPtr<IBaseFilter>& pDest);
	void						ReleaseFilter(CComPtr<IBaseFilter>& pbasefilter);
	

	bool						CreateAudioRender();
	bool						ConnectAudioRenderAuto();
	void						PrepareFilter(QVOD_FILTER_INFO& filter_info);
	void						RepalcePath(CString& path);
	//bool					GetQvodPlayerPath(TCHAR* path) ;

	bool						LoadComponents(QVOD_FILTER_INFO& filter_info, std::vector<HMODULE>&  modules);

	
	  
#ifdef SHOW_GRAPH_IN_GRAPHEDIT
	HRESULT		AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
	void		RemoveFromRot(DWORD pdwRegister);
#endif

protected:
	std::map<IBaseFilter*, std::vector<HMODULE>>	mModule;
	CComPtr<IGraphBuilder>			mpGraph;
	DWORD							mdwRegister;
	HWND							mhPlayWnd;
	FString							mSoundDevice;

	// INTERFACE
	CComPtr<IFFmpegMux>				mIFFmpegMux;
	//CComPtr<IQVODH265Encode>				mIQVODH265Encode;
	CComPtr<IPushSource>			mIPushSource;
	CComPtr<IVMRWindowlessControl>	mpVMRWindowless;

	// BASE FILTER
	CComPtr<IBaseFilter>			mPushSourceFilter;
	CComPtr<IBaseFilter>			mVideoMultiTeeFilter;	// 视频一路转多路过滤器
	CComPtr<IBaseFilter>			mAudioMultiTeeFilter;	// 音频一路转多路过滤器
	CComPtr<IBaseFilter>			mpFilterVideoRender;	// 视频Render
	CComPtr<IBaseFilter>			mpFilterAudioRender;	// 音频Render
	CComPtr<IBaseFilter>			mpCoreAVC;

	xEditPtr						mEdit;
	IDataSourcePtr					mDataSource;
};
