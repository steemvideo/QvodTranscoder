#pragma once
// interface IQvodCallBack
// {
// public:
// 	virtual long EventNotify(UINT uMsg, long wparam, long lparam) = 0;
// };
interface IQvodCallBack
{
public:
	virtual long EventNotify(UINT uMsg, long wparam, long lparam)=0;
};

#define QTC_MSG_BASE				0x100 + WM_USER

#define QTC_MSG_PROGRESS			QTC_MSG_BASE + 0x1     //转码时间进度 wparam为taskid，lparam为进度
#define QTC_MSG_PREVIEW_PROGRESS	QTC_MSG_BASE + 0x2     //预览时间进度 wparam为taskid，lparam为播放时间（ms）
#define QTC_MSG_TRANSCODE_COMPLETE  QTC_MSG_BASE + 0x3	   //转码完成 
#define QTC_MSG_TRANSCODE_ERROR		QTC_MSG_BASE + 0x4     //转码错误
#define QTC_MSG_FULLDISK_ERROR		QTC_MSG_BASE + 0x5     //转码错误