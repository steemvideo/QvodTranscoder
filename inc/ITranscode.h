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
//基本功能

	//创建任务
	virtual HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD& taskid) = 0;

	//创建预览任务
	virtual HRESULT CreatePreviewTask(DWORD taskid,  CComPtr<IPreview>& pPreview) = 0;

	//设置任务配置
	virtual HRESULT SetTaskConfig(DWORD taskid, const TRANSCODE_CONFIG& pConfig) = 0;

	//获取任务信息
	virtual const TRANSCODE_CONFIG_Ptr GetTaskConfig(DWORD taskid) = 0;

	//删除任务
	virtual HRESULT RemoveTask(DWORD taskid) = 0;

	//开始转码
	virtual HRESULT RunTask(DWORD taskid) = 0;

	//暂停转码
	virtual HRESULT PauseTask(DWORD taskid) = 0;

	//停止任务
	virtual HRESULT StopTask(DWORD taskid) = 0;

	//获取文件信息
	virtual const Qvod_DetailMediaInfo_Ptr GetMediaInfo(DWORD taskid) = 0;

	//获取当前转码任务数
	virtual DWORD GetTaskCount() = 0;

	//获取当前转码视频编码器的线程数
	virtual DWORD GetVideoCodecThreadCount() = 0;

	//设置当前转码视频编码器的线程数
	virtual HRESULT SetVideoCodecThreadCount(DWORD nthreadcount) = 0;

	//硬件加速转码？
	virtual bool	IsAccelTrans(DWORD taskid) = 0;
	//设置消息接收窗口句柄
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
	//创建任务
	virtual HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD taskid) = 0;

	//创建预览链路
	virtual HRESULT CreatePreviewTask(E_Play_Mode playmode, CComPtr<IPreview>& pPreview) = 0;

	//设置任务配置
	virtual HRESULT SetTaskConfig(const TRANSCODE_CONFIG& pConfig) = 0;

	//获取任务信息
	virtual const TRANSCODE_CONFIG_Ptr GetTaskConfig() = 0;

	//删除任务
	virtual HRESULT DestroyTask(bool deletedst) = 0;

	//开始转码
	virtual HRESULT RunTask() = 0;

	//暂停转码
	virtual HRESULT PauseTask() = 0;

	//停止任务
	virtual HRESULT StopTask() = 0;

	//获取Taskid
	virtual DWORD GetTaskID() = 0;

	//获取当前转码视频编码器的线程数
	virtual DWORD GetVideoCodecThreadCount() = 0;

	//设置当前转码视频编码器的线程数
	virtual HRESULT SetVideoCodecThreadCount(DWORD nthreadcount) = 0;

	//设置设置taskmanager的回调函数
	virtual HRESULT SetCallBack(IQvodCallBack* pcallback) = 0;

	virtual const boost::shared_ptr<Qvod_DetailMediaInfo> GetMediaInfo() = 0;
};