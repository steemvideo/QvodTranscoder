#pragma once
#include "IDataSource.h"
#include "IEdit.h"

class xEdit : public IDataSource, public IEdit
{
public:
	xEdit(void);
	~xEdit(void);

//IEditSource impl
public:
	// 获取编辑模块输出流信息
	virtual void		GetVideoStreamInfo(VIDEO_STREAM_INFO& video_stream_info);
	virtual void		GetAudioStreamInfo(AUDIO_STREAM_INFO& audio_stream_info);
	virtual void			SetStatus(EDIT_STATUS);
	virtual EDIT_STATUS		GetStatus();
	virtual void			Seek(XREFTIME);
	virtual XREFTIME		GetDuration();
	virtual xObjectPtr		GetVideoResultBuffer();
	virtual void			FreeVideoResultBuffer(xObjectPtr);
	virtual xObjectPtr		GetAudioResultBuffer();
	virtual void			FreeAudioResultBuffer(xObjectPtr);

	virtual	IEdit*			GetIEdit();
//IEdit impl
public:
	virtual ITrackPtr		AddTrack(TRACK_TYPE);
	virtual bool			DelTrack(ITrackPtr);
	
	virtual ITrackPtr		GetTrack(TRACK_TYPE, int);
	virtual int				GetTrackCount(TRACK_TYPE);
	virtual int				GetTrackID(ITrackPtr);

protected:
	class impl;
	boost::shared_ptr<impl>	this_;

};

typedef boost::shared_ptr<xEdit>	xEditPtr;
typedef boost::weak_ptr<xEdit>		xEditPtr_;