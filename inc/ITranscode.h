#pragma once
#include "TranscodeStruct.h"
#include "MessageDefine.h"
enum E_Play_Mode
{
	e_Play,
	e_Preview
};


interface IPreview:public IUnknown
{
	virtual HRESULT SetClipWindow(HWND hwnd) = 0;
	virtual HRESULT Play() = 0;
	virtual HRESULT Pause() = 0;
	virtual HRESULT Stop() = 0;
	virtual HRESULT GetDuration(REFERENCE_TIME& duration) = 0;
	virtual HRESULT GetCurPos(REFERENCE_TIME& pos) = 0;
	virtual HRESULT Seek(REFERENCE_TIME time) = 0;
	virtual HRESULT GetVolume(long * volume) = 0;
	virtual HRESULT SetVolume(long volume) = 0;
	virtual HRESULT GetCurrentImage(BYTE* pbuf, DWORD& len) = 0;
};

//class IQvodCallBack
//{
//public:
//	virtual long EventNotify(UINT task_id, long param_type, long param)=0;
//};

typedef boost::shared_ptr<TRANSCODE_CONFIG>	TRANSCODE_CONFIG_Ptr;
typedef boost::weak_ptr<TRANSCODE_CONFIG>	TRANSCODE_CONFIG_Ptr_;
typedef boost::shared_ptr<Qvod_DetailMediaInfo>	Qvod_DetailMediaInfo_Ptr;
typedef boost::weak_ptr<Qvod_DetailMediaInfo>	Qvod_DetailMediaInfo_Ptr_;

class ITaskManager
{
public:
//��������

	//��������
	virtual HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD& taskid) = 0;

	//����Ԥ������
	virtual HRESULT CreatePreviewTask(DWORD taskid,  CComPtr<IPreview>& pPreview) = 0;

	//������������
	virtual HRESULT SetTaskConfig(DWORD taskid, const TRANSCODE_CONFIG& pConfig) = 0;

	//��ȡ������Ϣ
	virtual const TRANSCODE_CONFIG_Ptr GetTaskConfig(DWORD taskid) = 0;

	//ɾ������
	virtual HRESULT RemoveTask(DWORD taskid) = 0;

	//��ʼת��
	virtual HRESULT RunTask(DWORD taskid) = 0;

	//��ͣת��
	virtual HRESULT PauseTask(DWORD taskid) = 0;

	//ֹͣ����
	virtual HRESULT StopTask(DWORD taskid) = 0;

	//��ȡ�ļ���Ϣ
	virtual const Qvod_DetailMediaInfo_Ptr GetMediaInfo(DWORD taskid) = 0;

	//��ȡ��ǰת��������
	virtual DWORD GetTaskCount() = 0;

	//��ȡ��ǰת����Ƶ���������߳���
	virtual DWORD GetVideoCodecThreadCount() = 0;

	//���õ�ǰת����Ƶ���������߳���
	virtual HRESULT SetVideoCodecThreadCount(DWORD nthreadcount) = 0;

	//Ӳ������ת�룿
	virtual bool	IsAccelTrans(DWORD taskid) = 0;
	//������Ϣ���մ��ھ��
	virtual HRESULT SetCallBack(IQvodCallBack* pcallback) = 0;
};


//interface IMux
//{
//	virtual HRESULT SetMuxConfig(MUX_CONFIG* pMuxConfig) = 0;
//	virtual HRESULT GetMuxConfig(MUX_CONFIG* pMuxConfig) = 0;
//	virtual HRESULT SetCallBack(IQvodCallBack* callback) = 0;
//};

interface ITask
{
	//��������
	virtual HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD taskid) = 0;

	//����Ԥ����·
	virtual HRESULT CreatePreviewTask(E_Play_Mode playmode, CComPtr<IPreview>& pPreview) = 0;

	//������������
	virtual HRESULT SetTaskConfig(const TRANSCODE_CONFIG& pConfig) = 0;

	//��ȡ������Ϣ
	virtual const TRANSCODE_CONFIG_Ptr GetTaskConfig() = 0;

	//ɾ������
	virtual HRESULT DestroyTask(bool deletedst) = 0;

	//��ʼת��
	virtual HRESULT RunTask() = 0;

	//��ͣת��
	virtual HRESULT PauseTask() = 0;

	//ֹͣ����
	virtual HRESULT StopTask() = 0;

	//��ȡTaskid
	virtual DWORD GetTaskID() = 0;

	//��ȡ��ǰת����Ƶ���������߳���
	virtual DWORD GetVideoCodecThreadCount() = 0;

	//���õ�ǰת����Ƶ���������߳���
	virtual HRESULT SetVideoCodecThreadCount(DWORD nthreadcount) = 0;

	//��������taskmanager�Ļص�����
	virtual HRESULT SetCallBack(IQvodCallBack* pcallback) = 0;

	virtual const boost::shared_ptr<Qvod_DetailMediaInfo> GetMediaInfo() = 0;
};