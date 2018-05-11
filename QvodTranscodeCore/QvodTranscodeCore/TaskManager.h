#pragma once
#include "MediaInfo.h"
#include "MessageDefine.h"
#include "TranscodeTask.h"
#include <map>
using namespace std;
extern "C" __declspec(dllexport) bool InitQvodTaskManager(void** ppTaskManager);
extern "C" __declspec(dllexport) void DestroyQvodTaskManager(void* ppTaskManager);


class CTaskManager : public ITaskManager, public IQvodCallBack
{
public:
	CTaskManager(void);
public:
	~CTaskManager(void);

	//��������
	HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD& taskid);


	//����Ԥ������
	HRESULT CreatePreviewTask(DWORD taskid,  CComPtr<IPreview>& pPreview);

	//������������
	HRESULT SetTaskConfig(DWORD taskid, const TRANSCODE_CONFIG& pConfig);

	//��ȡ������Ϣ
	const TRANSCODE_CONFIG_Ptr GetTaskConfig(DWORD taskid);

	//ɾ������
	HRESULT RemoveTask(DWORD taskid);

	//��ʼת��
	HRESULT RunTask(DWORD taskid);

	//��ͣת��
	HRESULT PauseTask(DWORD taskid);

	//ֹͣ����
	HRESULT StopTask(DWORD taskid);

	////��ȡ��������ʱ��
	//HRESULT GetDuration(DWORD taskid, REFERENCE_TIME* pduration);

	//��ȡ�ļ���Ϣ
	const Qvod_DetailMediaInfo_Ptr GetMediaInfo(DWORD taskid);

	//��ȡ��ǰת����Ƶ���������߳���
	DWORD GetVideoCodecThreadCount();

	//���õ�ǰת����Ƶ���������߳���
	HRESULT SetVideoCodecThreadCount(DWORD nthreadcount);

	bool	IsAccelTrans(DWORD taskid);

	//������Ϣ���մ��ھ��
	HRESULT SetCallBack(IQvodCallBack* pcallback);

	//UI��Ϣ�ص�����
	long EventNotify(UINT task_id, long param_type, long param);


	DWORD GetTaskCount();
protected:
	//std::map<DWORD, boost::shared_ptr<ITask>> m_mTaskList;
	struct TaskItem{
		DWORD taskid;
		boost::shared_ptr<CTranscodeTask> pITask;
	};
	std::vector<TaskItem> m_vTaskList;
	CCritSec m_Lock;


	IQvodCallBack* m_pQvodCallBack;
	DWORD m_nThreadCount;
};
