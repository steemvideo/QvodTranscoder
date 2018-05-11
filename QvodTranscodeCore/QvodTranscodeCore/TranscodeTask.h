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

	//创建任务
	HRESULT CreateTranscodeTask(const wchar_t* src_url, DWORD taskid);

	//设置任务配置
	HRESULT SetTaskConfig(const TRANSCODE_CONFIG& pConfig);

	//获取任务信息
	const TRANSCODE_CONFIG_Ptr GetTaskConfig();

	//删除任务
	HRESULT DestroyTask(bool deletedst);

	//开始转码
	HRESULT RunTask();

	//暂停转码
	HRESULT PauseTask();

	//停止任务
	HRESULT StopTask();

	//获取Taskid
	DWORD GetTaskID();

	//设置设置taskmanager的回调函数
	HRESULT SetCallBack(IQvodCallBack* pcallback);

	////设置预览播放窗口句柄
	//HRESULT SetClipWindow(HWND hwnd);

	////播放
	//HRESULT Play();

	////暂停
	//HRESULT Pause();

	////停止
	//HRESULT Stop();

	////获取播放预览时长
	//HRESULT GetDuration(REFERENCE_TIME& duration);

	////获取当前预览位置
	//HRESULT GetCurPos(REFERENCE_TIME& pos);

	////预览定位
	//HRESULT Seek(REFERENCE_TIME time);

	////获取音量
	//HRESULT GetVolume(long * volume);

	////设置音量
	//HRESULT SetVolume(long volume);

	//获取当前转码视频编码器的线程数
	DWORD GetVideoCodecThreadCount();

	//设置当前转码视频编码器的线程数
	HRESULT SetVideoCodecThreadCount(DWORD nthreadcount);

	//截取当前播放图片
	//HRESULT GetCurrentImage(BYTE* pbuf, DWORD& len);
	BOOL  IsAccelTrans();

	//创建预览链路
	HRESULT CreatePreviewTask(E_Play_Mode playmode, CComPtr<IPreview>& pPreview);

	const  Qvod_DetailMediaInfo_Ptr GetMediaInfo();
protected:
	//创建任务源
	HRESULT CreateSource();


	//创建转码链路
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
