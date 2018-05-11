#include "PushPin.h"

CPushPin::CPushPin(CSource *pSource, HRESULT *phr):CSourceStream(NAME("Push Source"), phr, pSource, L"Output")
{
	m_pIReceive = NULL;
	m_nStreamid = 0;
	m_rtStartts = 0;
	m_rtCurts = 0;
}

CPushPin::~CPushPin(void)
{
}

HRESULT CPushPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock lock(m_pFilter->pStateLock());
	m_rtStartts = tStart;
	if(!ThreadExists()) return S_FALSE;
	return __super::DeliverNewSegment(tStart, tStop, dRate);
}

HRESULT CPushPin::FillBuffer(IMediaSample *pSamp)
{
	Sleep(1000);
	OutputDebugString(L"fillbuffer\n");
	return S_OK;
}

HRESULT CPushPin::CheckMediaType(const CMediaType *pMediaType)
{
	return S_OK;
}

HRESULT CPushPin::GetMediaType(int iPosition, __inout CMediaType *pMediaType)
{
	CAutoLock lock(m_pFilter->pStateLock());
	CheckPointer(pMediaType,E_POINTER); 
	if(iPosition < 0)
	{
		return E_INVALIDARG;
	}
	if(iPosition > 0)
	{
		return VFW_S_NO_MORE_ITEMS;
	}
	pMediaType->InitMediaType();
	pMediaType->SetType(&m_mt.majortype);
	pMediaType->SetSubtype(&m_mt.subtype);
	pMediaType->SetFormatType(&m_mt.formattype);
	//pMediaType->AllocFormatBuffer();
	return S_OK;
}

HRESULT CPushPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	HRESULT hr;
	CAutoLock lock(m_pFilter->pStateLock());
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*) m_mt.Format();

	// Ensure a minimum number of buffers
	if (pRequest->cBuffers == 0)
	{
		pRequest->cBuffers = 2;
	}
	pRequest->cbBuffer = 65536;

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pRequest, &Actual);
	if (FAILED(hr)) 
	{
		return hr;
	}

	// Is this allocator unsuitable?
	if (Actual.cbBuffer < pRequest->cbBuffer)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CPushPin::OnThreadCreate(void)
{
	return S_OK;
}

HRESULT CPushPin::OnThreadDestroy(void)
{
	return S_OK;
}

STDMETHODIMP CPushPin::Notify(IBaseFilter *pSelf, Quality q)
{
	return S_OK;
}

HRESULT CPushPin::SetStreamID(DWORD streamid)
{
	m_nStreamid = streamid;
	return S_OK;
}

DWORD CPushPin::GetStreamID()
{
	return m_nStreamid;
}

HRESULT CPushPin::ConfigMediaType(const CMediaType* pMediaType)
{
	CheckPointer(pMediaType, E_POINTER);
	CAutoLock lock(m_pFilter->pStateLock());

	m_mt.majortype = pMediaType->majortype;
	m_mt.subtype = pMediaType->subtype;
	m_mt.formattype = pMediaType->formattype;
	m_mt.AllocFormatBuffer(pMediaType->cbFormat);
	m_mt.cbFormat = pMediaType->cbFormat;
	memcpy(m_mt.pbFormat, pMediaType->pbFormat, pMediaType->cbFormat);

	return S_OK;
}

HRESULT CPushPin::SetDataSrc(IReceive* pIReceive)
{
	CheckPointer(pIReceive, E_POINTER);
	CAutoLock lock(m_pFilter->pStateLock());
	m_pIReceive = pIReceive;
	return S_OK;
}

REFERENCE_TIME CPushPin::GetCurrentPos()
{
	return m_rtCurts;
}