#pragma once
#include "itranscode.h"
#include "xTranscodeGraph.h"
#include "MessageDefine.h"
#include "PreviewTask.h"

class CTranscodeTask :
	public ITask, public IQvodCallBack
{
public:
	CTranscodeTask(void);
public:
	~CTranscodeTask(void);

	virtual  long EventNotify(UINT uMsg, long wparam, long lparam);

	//��������
	HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD taskid);

	//������������
	HRESULT SetTaskConfig(const TRANSCODE_CONFIG& pConfig);

	//��ȡ������Ϣ
	const TRANSCODE_CONFIG_Ptr GetTaskConfig();

	//ɾ������
	HRESULT DestroyTask(bool deletedst);

	//��ʼת��
	HRESULT RunTask();

	//��ͣת��
	HRESULT PauseTask();

	//ֹͣ����
	HRESULT StopTask();

	//��ȡTaskid
	DWORD GetTaskID();

	//��������taskmanager�Ļص�����
	HRESULT SetCallBack(IQvodCallBack* pcallback);

	////����Ԥ�����Ŵ��ھ��
	//HRESULT SetClipWindow(HWND hwnd);

	////����
	//HRESULT Play();

	////��ͣ
	//HRESULT Pause();

	////ֹͣ
	//HRESULT Stop();

	////��ȡ����Ԥ��ʱ��
	//HRESULT GetDuration(REFERENCE_TIME& duration);

	////��ȡ��ǰԤ��λ��
	//HRESULT GetCurPos(REFERENCE_TIME& pos);

	////Ԥ����λ
	//HRESULT Seek(REFERENCE_TIME time);

	////��ȡ����
	//HRESULT GetVolume(long * volume);

	////��������
	//HRESULT SetVolume(long volume);

	//��ȡ��ǰת����Ƶ���������߳���
	DWORD GetVideoCodecThreadCount();

	//���õ�ǰת����Ƶ���������߳���
	HRESULT SetVideoCodecThreadCount(DWORD nthreadcount);

	//��ȡ��ǰ����ͼƬ
	//HRESULT GetCurrentImage(BYTE* pbuf, DWORD& len);
	BOOL  IsAccelTrans();

	//����Ԥ����·
	HRESULT CreatePreviewTask(E_Play_Mode playmode, CComPtr<IPreview>& pPreview);

	const  Qvod_DetailMediaInfo_Ptr GetMediaInfo();
protected:
	//��������Դ
	HRESULT CreateSource();


	//����ת����·
	HRESULT CreateTranscode();

	void KeepAspectRatio(boost::shared_ptr<TRANSCODE_CONFIG> p);

protected:
	boost::shared_ptr<TRANSCODE_CONFIG> m_pTranscode_config;
	boost::shared_ptr<xTranscodeGraph>  m_pGraphBuilder;
	CComPtr<IGraphBuilder>				m_pGraph;
	CComPtr<IMediaControl>				m_pMediaControl;
	IQvodCallBack*						m_pQvodCallBack;
	DWORD								m_nTaskId;
	e_TRANSCODE_STATE					m_eState;
	DWORD								m_nThreadCount;
	boost::shared_ptr<Qvod_DetailMediaInfo>		m_pSourceMediaInfo;
	FString		mURL;
};
