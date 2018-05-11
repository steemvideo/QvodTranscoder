#pragma once


#define QTC_MSG_BASE  0x100 + WM_USER

#define QTC_MSG_PROGRESS  QTC_MSG_BASE + 0x1            //转码时间进度 wparam为taskid，lparam为进度
#define QTC_MSG_PREVIEW_PROGRESS  QTC_MSG_BASE + 0x2    //预览时间进度 wparam为taskid，lparam为播放时间（ms）
#define QTC_MSG_TRANSCODE_COMPLETE  QTC_MSG_BASE + 0x3  //转码完成