#pragma once
#include "streams.h"
#include "PushPin.h"
#include <atlcoll.h>
#include "IPushSource.h"

class CPushSource:public CSource, public IPushSource, public IMediaSeeking
{
public:
	CPushSource(IUnknown *pUnk, HRESULT *phr);
public:
	~CPushSource(void);


	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

	//BaseFilter 接口实现
	int   GetPinCount(void);
	CBasePin *GetPin(int n);
	HRESULT AddPin( DWORD streamid, CMediaType *pmediatype, IReceive* pIReceive);

	//virtual HRESULT Connect(IPin * pReceivePin,	const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Pause();
	STDMETHODIMP Stop();
	HRESULT StartStreaming(void);
	HRESULT StopStreaming(void);

	//设置任务id
	void SetTaskId(DWORD taskid);

	//获取任务id
	DWORD GetTaskId();

	//设置回调函数接口指针
	HRESULT SetCallBack(IQvodCallBack *pCallBack);

	//添加PushSource输入流
	HRESULT AddStream(CMediaType *pmediatype, IReceive* pIReceive, DWORD& streamid);

	//移除PushSource输入流
	HRESULT RemoveStream(DWORD streamid);

	//IMediaSeeking
	STDMETHODIMP GetCapabilities(DWORD* pCapabilities);
	STDMETHODIMP CheckCapabilities(DWORD* pCapabilities);
	STDMETHODIMP IsFormatSupported(const GUID* pFormat);
	STDMETHODIMP QueryPreferredFormat(GUID* pFormat);
	STDMETHODIMP GetTimeFormat(GUID* pFormat);
	STDMETHODIMP IsUsingTimeFormat(const GUID* pFormat);
	STDMETHODIMP SetTimeFormat(const GUID* pFormat);
	STDMETHODIMP GetDuration(LONGLONG* pDuration);
	STDMETHODIMP GetStopPosition(LONGLONG* pStop);
	STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
	STDMETHODIMP ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat);
	STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);
	STDMETHODIMP GetPositions(LONGLONG* pCurrent, LONGLONG* pStop);
	STDMETHODIMP GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest);
	STDMETHODIMP SetRate(double dRate);
	STDMETHODIMP GetRate(double* pdRate);
	STDMETHODIMP GetPreroll(LONGLONG* pllPreroll);

protected:
	CAutoPtrList<CPushPin> m_pOutputs;
	CAutoPtrList<CPushPin> m_pActiveOutputs;
	static DWORD m_nstreamid;
	DWORD m_ntaskid;
	IQvodCallBack* m_pCallBack;
};
