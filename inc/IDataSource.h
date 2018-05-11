#pragma once
#include "xObject.h"
#include "TranscodeStruct.h"

class IEdit;

struct tagStreamInfo
{
	int		buffer_size;
};

struct VIDEO_STREAM_INFO : public tagStreamInfo
{
	EVIDEO_CHROMA_FMT fmt;
	int width;
	int height;
	int bitrate;
	int framerate;
};

struct AUDIO_STREAM_INFO : public tagStreamInfo
{
	EAUDIO_PCM_FMT fmt;
	int samplingrate;
	int channel;
	int bitrate;
};

class IDataSource
{
public:
	// 获取编辑模块输出流信息
	virtual void		GetVideoStreamInfo(VIDEO_STREAM_INFO& video_stream_info) = 0;
	virtual void		GetAudioStreamInfo(AUDIO_STREAM_INFO& audio_stream_info) = 0;

	// 状态控制
	virtual void		SetStatus(EDIT_STATUS s)  = 0;
	virtual EDIT_STATUS	GetStatus() = 0;

	// SEEK DURATION
	virtual void		Seek(XREFTIME pos)  = 0;
	virtual XREFTIME	GetDuration()  = 0;

	// 获取合成的数据
	virtual xObjectPtr	GetVideoResultBuffer()  = 0;
	virtual void		FreeVideoResultBuffer(xObjectPtr)  = 0;

	virtual xObjectPtr	GetAudioResultBuffer()  = 0;
	virtual void		FreeAudioResultBuffer(xObjectPtr) = 0;
	virtual	IEdit*		GetIEdit()	= 0;
};
typedef boost::shared_ptr<IDataSource>	IDataSourcePtr;