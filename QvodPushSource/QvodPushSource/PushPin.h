#pragma once
#include "streams.h"

class IReceive
{
public:
	virtual HRESULT Receive() = 0;
};

class CPushPin: public CSourceStream
{
public:
	CPushPin(CSource *pSource, HRESULT *phr);
public:
	~CPushPin(void);

	HRESULT FillBuffer(IMediaSample *pSamp);
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, __inout CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT OnThreadCreate(void);
	HRESULT OnThreadDestroy(void);
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
	STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q);

	HRESULT SetStreamID(DWORD streamid);
	DWORD GetStreamID();
	HRESULT ConfigMediaType(const CMediaType* pMediaType);
	HRESULT SetDataSrc(IReceive* pIReceive);
	REFERENCE_TIME GetCurrentPos();

protected:
	DWORD m_nStreamid;
	interface IReceive* m_pIReceive;
	REFERENCE_TIME m_rtStartts;
	REFERENCE_TIME m_rtCurts;
};
