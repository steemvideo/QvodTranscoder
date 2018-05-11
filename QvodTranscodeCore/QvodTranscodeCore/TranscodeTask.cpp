#include "stdafx.h"
#include "TranscodeTask.h"
#include "streams.h"
#include "xUtility.h"
#include "MediaInfo.h"

CTranscodeTask::CTranscodeTask(void)
{
	m_pQvodCallBack = NULL;
	m_eState = E_TSC_INIT;
	m_nThreadCount = 1;
}

CTranscodeTask::~CTranscodeTask(void)
{
}

//��������
HRESULT CTranscodeTask::CreateTranscodeTask(const wchar_t* src_url, DWORD taskid)
{
	mURL = src_url;
	m_nTaskId = taskid;
	return S_OK;
}

//������������
HRESULT CTranscodeTask::SetTaskConfig(const TRANSCODE_CONFIG& config)
{
	if(!m_pTranscode_config)
	{
		m_pTranscode_config.reset(new TRANSCODE_CONFIG);
	}

	*m_pTranscode_config = config;
	return S_OK;
}

//��ȡ������Ϣ
const TRANSCODE_CONFIG_Ptr CTranscodeTask::GetTaskConfig()
{
	return m_pTranscode_config;
}

//ɾ������
HRESULT CTranscodeTask::DestroyTask(bool deletedst)
{
	if(m_pGraphBuilder.get())
	{
		m_pGraph.Release();
		m_pMediaControl.Release();
		m_pGraphBuilder->DestroyGraph();
		m_eState = E_TSC_INIT;
	}
	return S_OK;
}

//��ʼת��
HRESULT CTranscodeTask::RunTask()
{
	HRESULT hr = E_FAIL;
	if(m_eState != E_TSC_READY && m_eState != E_TSC_PAUSE)
	{
		if(FAILED(hr = CreateTranscode()))
		{
			xTranscodeGraph::Trace(-1, L"FAILED(hr = CreateTranscode())\n");
			return hr;
		}
	}

	if(m_pGraph && m_pMediaControl)
	{
		xTranscodeGraph::Trace(2, L"before m_pMediaControl->Run\n");
		//MessageBox(NULL, NULL, NULL, MB_TOPMOST);
		hr = m_pMediaControl->Run();
		//MessageBox(NULL, NULL, NULL, MB_TOPMOST);
		xTranscodeGraph::Trace(2, L"after m_pMediaControl->Run\n");
		if(SUCCEEDED(hr))
		{
			m_eState = E_TSC_RUNNING;
		}
		else
		{
			xTranscodeGraph::Trace(-2, L"m_pMediaControl->Run failed:hr:%d\n", hr);
		}
	}
	return hr;
}

//��ͣת��
HRESULT CTranscodeTask::PauseTask()
{
	HRESULT hr = E_FAIL;
	if(m_pGraph && m_pMediaControl)
	{
		hr = m_pMediaControl->Pause();
		if(SUCCEEDED(hr))
		{
			m_eState = E_TSC_PAUSE;
		}
		return hr;
	}
	return hr;
}

//ֹͣ����
HRESULT CTranscodeTask::StopTask()
{
	if(m_pGraph && m_pMediaControl && m_eState != E_TSC_STOP)
	{
		xTranscodeGraph::Trace(2,L"before StopTask\n");
		HRESULT hr = m_pMediaControl->Stop();
		xTranscodeGraph::Trace(2,L"after StopTask\n");
		if(SUCCEEDED(hr))
		{
			m_eState = E_TSC_STOP;
			xTranscodeGraph::Trace(2,L"before DestroyTask\n");
			DestroyTask(false);
			xTranscodeGraph::Trace(2,L"after DestroyTask\n");
		}
		return hr;
	}
	return E_FAIL;
}

//��ȡTaskid
DWORD CTranscodeTask::GetTaskID()
{
	return m_nTaskId;
}

//��������taskmanager�Ļص�����
HRESULT CTranscodeTask::SetCallBack(IQvodCallBack* pcallback)
{
	CheckPointer(pcallback, E_POINTER);
	m_pQvodCallBack = pcallback;
	return S_OK;
}

////����Ԥ�����Ŵ��ھ��
//HRESULT CTranscodeTask::SetClipWindow(HWND hwnd)
//{
//	m_hwnd = hwnd;
//	return S_OK;
//}
//
////����
//HRESULT CTranscodeTask::Play()
//{
//	return S_OK;
//}
//
////��ͣ
//HRESULT CTranscodeTask::Pause()
//{
//	return S_OK;
//}
//
////ֹͣ
//HRESULT CTranscodeTask::Stop()
//{
//	return S_OK;
//}
//
////��ȡ����Ԥ��ʱ��
//HRESULT CTranscodeTask::GetDuration(REFERENCE_TIME& duration)
//{
//	return S_OK;
//}
//
////��ȡ��ǰԤ��λ��
//HRESULT CTranscodeTask::GetCurPos(REFERENCE_TIME& pos)
//{
//	return S_OK;
//}
//
////Ԥ����λ
//HRESULT CTranscodeTask::Seek(REFERENCE_TIME time)
//{
//	return S_OK;
//}

//��������Դ
HRESULT CTranscodeTask::CreateSource()
{
	return S_OK;
}

////��ȡ����
//HRESULT CTranscodeTask::GetVolume(long * volume)
//{
//	return S_OK;
//}
//
////��������
//HRESULT CTranscodeTask::SetVolume(long volume)
//{
//	return S_OK;
//}


//��ȡ��ǰת����Ƶ���������߳���
DWORD CTranscodeTask::GetVideoCodecThreadCount()
{
	return m_nThreadCount;
}

//���õ�ǰת����Ƶ���������߳���
HRESULT CTranscodeTask::SetVideoCodecThreadCount(DWORD nthreadcount)
{
	m_nThreadCount = nthreadcount;
	return S_OK;
}

BOOL CTranscodeTask::IsAccelTrans()
{
	return m_pGraphBuilder->IsAccelTrans();
}

//��ȡ��ǰ����ͼƬ
 //HRESULT CTranscodeTask::GetCurrentImage(BYTE* pbuf, DWORD& len)
 //{
 //	//return m_pPreviewTask->GetCurrentImage(pbuf, len);
	// return E_NOTIMPL;
 //}

//����Ԥ����·
HRESULT CTranscodeTask::CreatePreviewTask(E_Play_Mode playmode, CComPtr<IPreview>& pPreview)
{
	/*if(m_pTranscode_config.get() == NULL)
	{
		return E_FAIL;
	}*/

	/*if(m_pPreviewBuilder == NULL)
	{
		m_pPreviewBuilder.reset(new xTranscodeGraph());
	}*/

	xTranscodeGraph::build_graph_parameter param;
	param.bNeedVideo = true;/*m_pTranscode_config->VideoCodec.bitrate ? true : false;*/
	param.bNeedAudio = false;/*m_pTranscode_config->AudioCodec.bitrate ? true : false;*/
// 	param.dstWidth = m_pTranscode_config->VideoCodec.width;
// 	param.dstHeight = m_pTranscode_config->VideoCodec.height;
	param.bNeedSubTitle = false;
	param.srcurl = mURL;

// 	std::vector<CLIP_CONFIG>::iterator it = m_pTranscode_config->ClipConfig_List.begin();
// 	for(; it != m_pTranscode_config->ClipConfig_List.end(); it++)
// 	{
// 		if(it->bVideoenable)
// 		{
// 			param.srcurl = mURL;
// 			param.dstcurl = m_pTranscode_config->DestURL;
// 			param.nAudioStreamID = 0;
// 			param.AudioVolume = m_pTranscode_config->AudioVolume;
// 		}
// 		else if(it->bSubtitleenable)
// 		{
// 			param.bNeedSubTitle = true;
// 			param.subpath = it->filepath;
// 		}
// 
// 	}

	if (!m_pSourceMediaInfo)
	{
		m_pSourceMediaInfo.reset(new Qvod_DetailMediaInfo);

		CMediaInfo::GetInstance()->GetDetailMediaInfo(mURL.c_str(), m_pSourceMediaInfo);
	}
	param.media_info = *m_pSourceMediaInfo;

	CPreviewTask* _pPreview = new CPreviewTask();
	
	E_GraphType graphtype = e_GraphPreview;
	if(FAILED(_pPreview->BuildGraph(param, playmode)))
	{
		return E_FAIL;
	}
	
	pPreview = _pPreview;
	return S_OK;
}

//����ת����·
HRESULT CTranscodeTask::CreateTranscode()
{
	if(m_pTranscode_config.get() == NULL)
	{
		return E_FAIL;
	}

	if(m_eState == E_TSC_RUNNING || m_eState == E_TSC_PAUSE)
	{
		return S_FALSE;
	}

	if(m_pGraphBuilder == NULL)
	{
		m_pGraphBuilder.reset(new xTranscodeGraph());
	}

	xTranscodeGraph::build_graph_parameter param;
	REFERENCE_TIME rtStart = 0, rtStop = 0;
	param.bNeedVideo = m_pTranscode_config->VideoCodec.bitrate ? true : false;
	param.bNeedAudio = m_pTranscode_config->AudioCodec.bitrate ? true : false;
	param.dstWidth = m_pTranscode_config->VideoCodec.width;
	param.dstHeight = m_pTranscode_config->VideoCodec.height;
	param.bAccel = m_pTranscode_config->Accel;
	param.dstVideo = m_pTranscode_config->VideoCodec.CodecType;

	std::vector<CLIP_CONFIG>::iterator it = m_pTranscode_config->ClipConfig_List.begin();
	for(; it != m_pTranscode_config->ClipConfig_List.end(); it++)
	{
		if(it->bVideoenable)
		{
			param.srcurl = mURL;
			param.dstcurl = m_pTranscode_config->DestURL;
			param.nAudioStreamID = 0;
			param.AudioVolume = m_pTranscode_config->AudioVolume;
			ASSERT(it->dststartts <= it->dststopts);
			rtStart = it->srcstartts;
			rtStop = it->srcstopts;
		}
		else if(it->bSubtitleenable)
		{
			param.bNeedSubTitle = true;
			param.subpath = it->filepath;
		}
		
	}

	if (!m_pSourceMediaInfo)
	{
		m_pSourceMediaInfo.reset(new Qvod_DetailMediaInfo);

		CMediaInfo::GetInstance()->GetDetailMediaInfo(mURL.c_str(), m_pSourceMediaInfo);
	}
	param.media_info = *m_pSourceMediaInfo;

	if(param.AudioVolume < 0 || param.AudioVolume > 1000)
	{
		param.AudioVolume = 100;
	}

	m_pGraph = m_pGraphBuilder->BuildGraph(param);
	if(m_pGraph == NULL)
	{
		return E_FAIL;
	}

	boost::shared_ptr<TRANSCODE_CONFIG> pconfig(new TRANSCODE_CONFIG);
	*pconfig = *m_pTranscode_config;	
	//if(pconfig->ContainerType != _E_CON_3GP)//3gp�����ֱַ���
	KeepAspectRatio(pconfig);
// 	m_pGraphBuilder->GetIFFmpegMux();
// 


	//---------------------------------���ñ������-----------------------------------
 	//CComPtr<IFFmpegMux>	 iffmpegmux(m_pGraphBuilder->GetIFFmpegMux());
//  	if(iffmpegmux == NULL)
//  	{
//  		return E_FAIL;
//  	}
 
 	MUX_CONFIG_EX muxconfig;
 	muxconfig.AudioCodec = pconfig->AudioCodec;
 	muxconfig.VideoCodec = pconfig->VideoCodec;
	muxconfig.VideoCodec.framerate = pconfig->VideoCodec.framerate;
 	muxconfig.ContainerType = pconfig->ContainerType;
 	muxconfig.AudioVolume = param.AudioVolume;
	muxconfig.maxThreadCount = m_nThreadCount;
	muxconfig.rtStart = rtStart;
	muxconfig.rtStop = rtStop;
	muxconfig.callBack = dynamic_cast<IQvodCallBack*>(this);


 	if(FAILED(m_pGraphBuilder->SetMuxConfigure(muxconfig, param.media_info.videoInfo[0]->nFrameRate)))
	{
		DestroyTask(true);
		return E_FAIL;
	}
	
	m_eState = E_TSC_READY;

	return m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
	
}

void CTranscodeTask::KeepAspectRatio(boost::shared_ptr<TRANSCODE_CONFIG> p)
{
	if(p == NULL ||!p->KeepAspectRatio || p->ContainerType == _E_CON_3GP)
	{
		return;
	}
	else if(p->VideoCodec.width > 0 && p->VideoCodec.height > 0)
	{
		GetMediaInfo();
		long width = 0, height = 0, w = 0, h = 0;
		if(m_pSourceMediaInfo->nVideoCount > 0)
		{
			width = m_pSourceMediaInfo->videoInfo[0]->nWidth;
			height = m_pSourceMediaInfo->videoInfo[0]->nHeight;
			h = p->VideoCodec.width  * height/ width;
			if( h <= m_pTranscode_config->VideoCodec.height)
			{
				p->VideoCodec.height = h;
			}
			else
			{
				p->VideoCodec.width = p->VideoCodec.height * width / height;
			}
		}
	}

	//��߱���Ϊ2�ı���������ת����ʧ��
	if(p->VideoCodec.width%2 != 0)
	{
		p->VideoCodec.width++;
	}

	if(p->VideoCodec.height%2 != 0)
	{
		p->VideoCodec.height++;
	}
// 	wchar_t str[100];
// 	swprintf(str, L"������Ƶ��߱Ⱥ�width: %d,  height:%d\n", p->VideoCodec.width, p->VideoCodec.height);
// 	OutputDebugString(str);
}

const boost::shared_ptr<Qvod_DetailMediaInfo> CTranscodeTask::GetMediaInfo()
{
	BOOL bOK = TRUE;
	if (!m_pSourceMediaInfo)
	{
		m_pSourceMediaInfo.reset(new Qvod_DetailMediaInfo);

		bOK = CMediaInfo::GetInstance()->GetDetailMediaInfo(mURL.c_str(), m_pSourceMediaInfo);
	}
	return m_pSourceMediaInfo;
}

long CTranscodeTask::EventNotify(UINT uMsg, long wparam, long lparam)
{
	if(m_pQvodCallBack)
	{
		return m_pQvodCallBack->EventNotify(uMsg, m_nTaskId, lparam);
	}
	return -1;
}