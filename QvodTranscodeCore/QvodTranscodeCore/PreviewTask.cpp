#include "StdAfx.h"
#include "PreviewTask.h"

CPreviewTask::CPreviewTask(void):CUnknown(NAME("Preview task"),NULL)
{
	m_ePlayState = e_stop;
}

CPreviewTask::~CPreviewTask(void)
{
	m_pPreviewGraph.Release();
	m_pMediaControl.Release();
	m_pIVideoWindow.Release();
	m_pMediaSeeking.Release();
	m_pBasicAudio.Release();

	if(m_pPreviewBuilder)
	{
		m_pPreviewBuilder->DestroyGraph();
	}
}

HRESULT CPreviewTask::BuildGraph(xTranscodeGraph::build_graph_parameter& param, E_Play_Mode playmode)
{
	if(m_pPreviewBuilder == NULL)
	{
		m_pPreviewBuilder.reset(new xTranscodeGraph());
	}

	E_GraphType Graphtype = e_GraphPreview;
	if(playmode == e_Play)
	{
		Graphtype = e_GraphPlay;
	}
	else if(playmode == e_Preview)
	{
		Graphtype = e_GraphPreview;
	}

	m_pPreviewGraph = m_pPreviewBuilder->BuildGraph(param, Graphtype);

	if(m_pPreviewGraph == NULL)
	{
		return E_FAIL;
	}

	if(FAILED(m_pPreviewGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl)) || m_pMediaControl == NULL)
	{
		return E_FAIL;
	}

	m_pPreviewGraph->QueryInterface(IID_IVideoWindow, (void**)&m_pIVideoWindow);

	m_pPreviewGraph->QueryInterface(IID_IMediaSeeking, (void**)&m_pMediaSeeking);
	m_pPreviewGraph->QueryInterface(IID_IBasicAudio, (void**)& m_pBasicAudio);
	return S_OK;
}

HRESULT CPreviewTask::SetClipWindow(HWND hwnd)
{
	/*if(m_pIVideoWindow)
	{
		m_pIVideoWindow->put_Owner(hwnd);
		m_pIVideoWindow->put_WindowStyle(WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_pIVideoWindow->put_MessageDrain(hwnd);
	}
	return S_OK;*/
	return S_OK;
}

HRESULT CPreviewTask::Play()
{
	if(m_pMediaControl)
	{
		if(SUCCEEDED(m_pMediaControl->Run()))
		{
			m_ePlayState = e_run;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CPreviewTask::Pause()
{
	if(m_pMediaControl)
	{
		if(SUCCEEDED(m_pMediaControl->Pause()))
		{
			m_ePlayState = e_pause;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CPreviewTask::Stop()
{
	if(m_pMediaControl)
	{
		if(SUCCEEDED(m_pMediaControl->Stop()))
		{
			m_ePlayState = e_stop;
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT CPreviewTask::GetDuration(REFERENCE_TIME& duration)
{
	if(m_pMediaSeeking)
	{
		return m_pMediaSeeking->GetDuration(&duration);
	}
	return E_FAIL;
}

HRESULT CPreviewTask::GetCurPos(REFERENCE_TIME& pos)
{
	if(m_pMediaSeeking)
	{
		return m_pMediaSeeking->GetCurrentPosition(&pos);
	}
	return E_FAIL;
}

HRESULT CPreviewTask::Seek(REFERENCE_TIME time)
{
	if(m_pMediaSeeking)
	{
		return m_pMediaSeeking->SetPositions(&time, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	}
	return E_FAIL;
}

HRESULT CPreviewTask::GetVolume(long * volume)
{
	CheckPointer(volume, E_INVALIDARG);
	if(m_pBasicAudio)
	{
		return m_pBasicAudio->get_Volume(volume);
	}
	return E_FAIL;
}

HRESULT CPreviewTask::SetVolume(long volume)
{
	CheckPointer(volume, E_INVALIDARG);
	if(m_pBasicAudio)
	{
		return m_pBasicAudio->put_Volume(volume);
	}
	return E_FAIL;
}

HRESULT CPreviewTask::GetCurrentImage(BYTE* pbuf, DWORD& len)
{
	HRESULT hr = E_FAIL;
	if(m_pPreviewBuilder && m_pPreviewBuilder->GetIPostVideoEx())
	{
		CComPtr<IPostVideoEffectEx> ppostvideo = m_pPreviewBuilder->GetIPostVideoEx();
		if(!ppostvideo)
			return E_FAIL;

		BITMAPINFOHEADER Bih;
		if(SUCCEEDED(ppostvideo->SnapShot(1000, &Bih, NULL)))
		{
			if(Bih.biSizeImage + sizeof(BITMAPINFOHEADER) > len)
			{
				if (!Bih.biSizeImage)
					Bih.biSizeImage = Bih.biWidth * abs(Bih.biHeight) * 4; 
				len = Bih.biSizeImage + sizeof(BITMAPINFOHEADER);
				return S_FALSE;
			}
		}
		CheckPointer(pbuf, E_INVALIDARG);

		memcpy(pbuf,&Bih, sizeof(Bih));
		if(ppostvideo)
		{
			return ppostvideo->GetSnapshotBuffer(pbuf+ sizeof(Bih), len - sizeof(Bih));
		}
	}
	return hr;
}