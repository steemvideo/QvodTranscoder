#pragma once
#include "xobject.h"
#include "TranscodeStruct.h"

class   xAudioBuffer : public xObject
{
public:	
	struct bufferPortal
	{
		EAUDIO_PCM_FMT	fmt;
		int			freq;	   
		void*       ptr;
		int			dwDataSize,dwBufferSize;
		byte        channels;
	};
	xAudioBuffer( int buffer_size, byte nChannel, int  nfrequency, EAUDIO_PCM_FMT  eFmt);
	void        Map( bufferPortal* pPortal );
	void		UnMap(int data_size);	
protected:
	class impl;
	boost::shared_ptr<impl>  this_;	
};

typedef boost::shared_ptr<xAudioBuffer>	xAudioBufferPtr;
typedef boost::weak_ptr<xAudioBuffer>	xAudioBufferPtr_;