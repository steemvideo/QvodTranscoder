#include "PushSource.h"

//DEFINE_GUID(<<CLSID_PUSH_SOURCE>>, 
//			0x16bcd081, 0x303d, 0x4d52, 0xb3, 0x16, 0xde, 0x89, 0x14, 0xdd, 0x6, 0x70);

DWORD CPushSource::m_nstreamid(0);
CPushSource::CPushSource(IUnknown *pUnk, HRESULT *phr):CSource(L"Push Source", pUnk, CLSID_PUSH_SOURCE)
{
	m_nstreamid = 0;
	m_ntaskid = 0;
	m_pCallBack = NULL;
	
	AddPin(m_nstreamid++, NULL, NULL);
	AddPin(m_nstreamid++, NULL, NULL);
}

CPushSource::~CPushSource(void)
{
}

CUnknown * WINAPI CPushSource::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
	CPushSource *pPushSouce = new CPushSource(pUnk, phr);
	if(pPushSouce == NULL)
	{
		*phr = E_OUTOFMEMORY;
	}
	else
	{
		*phr = S_OK;
	}
	return pPushSouce;
}

STDMETHODIMP CPushSource::NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv)
{
	if(riid == IID_IPushSource)
	{
		return GetInterface((IPushSource*)this, ppv);
	}
	else if (riid == IID_ISpecifyPropertyPages)
	{
		HRESULT hr = GetInterface((ISpecifyPropertyPages *) this, ppv);
		return hr;
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}

//BaseFilter 接口实现
int CPushSource::GetPinCount(void)
{
	CAutoLock lck(&m_cStateLock);
	return m_iPins;
}
CBasePin* CPushSource::GetPin(int n)
{
	CAutoLock lck(&m_cStateLock);

	POSITION pos = m_pOutputs.GetHeadPosition();
	int i = 0;

	while(pos && i++ != n)m_pOutputs.GetNext(pos);
	if(pos)
	{
		return m_pOutputs.GetAt(pos);
	}
	return NULL;
}

HRESULT CPushSource::AddPin(DWORD streamid, CMediaType *pmediatype, IReceive* pIReceive)
{
	CAutoLock lck(&m_cStateLock);

	HRESULT hr = S_OK;
	CAutoPtr<CPushPin> prt(new CPushPin(this, &hr));
	prt->SetStreamID(streamid);
	//prt->ConfigMediaType(pmediatype);
	//prt->SetDataSrc(pIReceive);
	m_pOutputs.AddTail(prt);
	return S_OK;
}

//HRESULT CPushSource::Connect(IPin * pReceivePin,	const AM_MEDIA_TYPE *pmt)
//{
//	return S_OK;
//}

STDMETHODIMP CPushSource::Run(REFERENCE_TIME tStart)
{
	CAutoLock lck(&m_cStateLock);
	return CBaseFilter::Run(tStart);
}

STDMETHODIMP CPushSource::Pause()
{
	CAutoLock lck(&m_cStateLock);
	HRESULT hr = NOERROR;
	if (m_State == State_Paused) 
	{
		// (This space left deliberately blank)
	}
	else 
	{
		if (m_State == State_Stopped) 
		{
			// allow a class derived from CTransformFilter
			// to know about starting and stopping streaming
			hr = StartStreaming();
		}

		if (SUCCEEDED(hr)) 
		{
			hr = CBaseFilter::Pause();
		}
	}
	return hr;
}

STDMETHODIMP CPushSource::Stop()
{
	CAutoLock lck1(&m_cStateLock);

	HRESULT hr = CBaseFilter::Stop();
	if (SUCCEEDED(hr))
	{
		hr = StopStreaming();
	}

	return hr;
}

HRESULT CPushSource::StartStreaming(void)
{
	CAutoLock lck(&m_cStateLock);
	return S_OK;
}

HRESULT CPushSource::StopStreaming(void)
{
	CAutoLock lck(&m_cStateLock);
	return S_OK;
}

//设置任务id
void CPushSource::SetTaskId(DWORD taskid)
{
	m_ntaskid = taskid;
}

//获取任务id
DWORD CPushSource::GetTaskId()
{
	return m_ntaskid;
}

//设置回调函数接口指针
HRESULT CPushSource::SetCallBack(IQvodCallBack *pCallBack)
{
	CheckPointer(pCallBack, E_POINTER);
	m_pCallBack = pCallBack;
	return S_OK;
}

HRESULT CPushSource::AddStream(CMediaType *pmediatype, IReceive* pIReceive, DWORD& streamid)
{
	CAutoLock lck(&m_cStateLock);
	CheckPointer(pmediatype, E_POINTER);
	CheckPointer(pIReceive, E_POINTER);
	streamid = m_nstreamid++;
	return AddPin(streamid, pmediatype, pIReceive);
}

HRESULT CPushSource::RemoveStream(DWORD streamid)
{
	CAutoLock lck(&m_cStateLock);

	POSITION pos = m_pOutputs.GetHeadPosition();
	while(pos && m_pOutputs.GetAt(pos)->GetStreamID() != streamid)m_pOutputs.GetNext(pos);
	if(pos)
	{
		m_pOutputs.RemoveAt(pos);
		return S_OK;
	}

	return E_INVALIDARG;
}

STDMETHODIMP CPushSource::GetCapabilities(DWORD* pCapabilities)
{
	return pCapabilities ? *pCapabilities = 
		AM_SEEKING_CanGetStopPos|
		AM_SEEKING_CanGetDuration|
		AM_SEEKING_CanSeekAbsolute|
		AM_SEEKING_CanSeekForwards|
		AM_SEEKING_CanSeekBackwards, S_OK : E_POINTER;
}

STDMETHODIMP CPushSource::CheckCapabilities(DWORD* pCapabilities)
{
	CheckPointer(pCapabilities, E_POINTER);
	if(*pCapabilities == 0) return S_OK;
	DWORD caps;
	GetCapabilities(&caps);
	if((caps&*pCapabilities) == 0) return E_FAIL;
	if(caps == *pCapabilities) return S_OK;
	return S_FALSE;
}

STDMETHODIMP CPushSource::IsFormatSupported(const GUID* pFormat)
{
	return !pFormat ? E_POINTER : *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

STDMETHODIMP CPushSource::QueryPreferredFormat(GUID* pFormat)
{
	return GetTimeFormat(pFormat);
}

STDMETHODIMP CPushSource::GetTimeFormat(GUID* pFormat)
{
	return pFormat ? *pFormat = TIME_FORMAT_MEDIA_TIME, S_OK : E_POINTER;
}

STDMETHODIMP CPushSource::IsUsingTimeFormat(const GUID* pFormat)
{
	return IsFormatSupported(pFormat);
}

STDMETHODIMP CPushSource::SetTimeFormat(const GUID* pFormat)
{
	return S_OK == IsFormatSupported(pFormat) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CPushSource::GetDuration(LONGLONG* pDuration)
{
	*pDuration = 0;
	return S_OK;
}

STDMETHODIMP CPushSource::GetStopPosition(LONGLONG* pStop)
{
	return GetDuration(pStop);
}

STDMETHODIMP CPushSource::GetCurrentPosition(LONGLONG* pCurrent)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPushSource::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPushSource::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
	return S_OK;
}

STDMETHODIMP CPushSource::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
	CAutoLock lck(&m_cStateLock);
	if(m_State == State_Stopped)
	{
		return E_FAIL;
	}
	POSITION pos = m_pActiveOutputs.GetHeadPosition();
	if(pos == NULL)
	{
		*pCurrent = 0;
		return S_FALSE;
	}

	LONGLONG curpos = _I64_MAX;
	while(pos && m_pActiveOutputs.GetAt(pos)->GetCurrentPos() <= curpos)
	{
		curpos = m_pActiveOutputs.GetAt(pos)->GetCurrentPos();
		m_pActiveOutputs.GetNext(pos);
	}
	*pCurrent = curpos;
	return S_OK;
}

STDMETHODIMP CPushSource::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
	if(pEarliest) *pEarliest = 0;
	return GetDuration(pLatest);
}

STDMETHODIMP CPushSource::SetRate(double dRate)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPushSource::GetRate(double* pdRate)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPushSource::GetPreroll(LONGLONG* pllPreroll)
{
	return E_NOTIMPL;
}