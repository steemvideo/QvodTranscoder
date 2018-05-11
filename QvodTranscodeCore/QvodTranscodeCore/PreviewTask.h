#pragma once
#include "../../inc/ITranscode.h"
#include "xTranscodeGraph.h"
enum e_play_state
{
	e_run,
	e_pause,
	e_stop

};

class CPreviewTask:public IPreview, public CUnknown
{
public:
	CPreviewTask(void);
public:
	~CPreviewTask(void);

	DECLARE_IUNKNOWN
	/*STDMETHODIMP QueryInterface(REFIID riid, __deref_out void **ppv) {
		return GetOwner()->QueryInterface(riid,ppv);
	};
	STDMETHODIMP_(ULONG) AddRef() {
	return GetOwner()->AddRef();
	};
	STDMETHODIMP_(ULONG) Release() {
	return GetOwner()->Release();
	};*/
	HRESULT BuildGraph(xTranscodeGraph::build_graph_parameter& param, E_Play_Mode playmode);
	HRESULT SetClipWindow(HWND hwnd);
	HRESULT Play();
	HRESULT Pause();
	HRESULT Stop();
	HRESULT GetDuration(REFERENCE_TIME& duration);
	HRESULT GetCurPos(REFERENCE_TIME& pos);
	HRESULT Seek(REFERENCE_TIME time);
	HRESULT GetVolume(long * volume);
	HRESULT SetVolume(long volume);
	HRESULT GetCurrentImage(BYTE* pbuf, DWORD& len);

protected:
	boost::shared_ptr<xTranscodeGraph>  m_pPreviewBuilder;
	CComPtr<IGraphBuilder>				m_pPreviewGraph;
	CComPtr<IMediaControl>				m_pMediaControl;
	CComPtr<IVideoWindow>				m_pIVideoWindow;
	CComPtr<IMediaSeeking>				m_pMediaSeeking;
	CComPtr<IBasicAudio>				m_pBasicAudio;
	e_play_state						m_ePlayState;
};
