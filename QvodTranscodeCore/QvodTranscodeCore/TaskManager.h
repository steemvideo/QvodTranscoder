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

	//创建任务
	HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD& taskid);


	//创建预览任务
	HRESULT CreatePreviewTask(DWORD taskid,  CComPtr<IPreview>& pPreview);

	//设置任务配置
	HRESULT SetTaskConfig(DWORD taskid, const TRANSCODE_CONFIG& pConfig);

	//获取任务信息
	const TRANSCODE_CONFIG_Ptr GetTaskConfig(DWORD taskid);

	//删除任务
	HRESULT RemoveTask(DWORD taskid);

	//开始转码
	HRESULT RunTask(DWORD taskid);

	//暂停转码
	HRESULT PauseTask(DWORD taskid);

	//停止任务
	HRESULT StopTask(DWORD taskid);

	////获取任务运行时间
	//HRESULT GetDuration(DWORD taskid, REFERENCE_TIME* pduration);

	//获取文件信息
	const Qvod_DetailMediaInfo_Ptr GetMediaInfo(DWORD taskid);

	//获取当前转码视频编码器的线程数
	DWORD GetVideoCodecThreadCount();

	//设置当前转码视频编码器的线程数
	HRESULT SetVideoCodecThreadCount(DWORD nthreadcount);

	bool	IsAccelTrans(DWORD taskid);

	//设置消息接收窗口句柄
	HRESULT SetCallBack(IQvodCallBack* pcallback);

	//UI消息回调函数
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
