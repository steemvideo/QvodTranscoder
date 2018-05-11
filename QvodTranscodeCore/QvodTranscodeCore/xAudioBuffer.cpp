#include "StdAfx.h"
#include "xAudioBuffer.h"
#include "xUtility.h"

class xAudioBuffer::impl
{
public:
	impl(int buffer_size, byte nChannel, int  nfrequency, EAUDIO_PCM_FMT  eFmt)
		:mFmt(eFmt)
		,mFreq(nfrequency)
		,mDataSize(0)
		,mBufferSize(buffer_size)
		,mChannels(nChannel)
	{
		mpDataBuffer.reset(new byte[buffer_size]);
	}

	boost::shared_array<byte>  mpDataBuffer;
	EAUDIO_PCM_FMT	mFmt;
	int			mFreq;
	int			mDataSize;
	int			mBufferSize;
	byte        mChannels;
};

xAudioBuffer::xAudioBuffer( int buffer_size, byte nChannel, int  nfrequency, EAUDIO_PCM_FMT  eFmt)
: this_(new impl(buffer_size, nChannel, nfrequency, eFmt))
{
	
}

void xAudioBuffer::Map( bufferPortal* pPortal )
{
	pPortal->ptr          = this_->mpDataBuffer.get();
	pPortal->dwBufferSize = this_->mBufferSize;;
	pPortal->dwDataSize   = this_->mDataSize;
	pPortal->channels     = this_->mChannels;
	pPortal->fmt          = this_->mFmt;
	pPortal->freq         = this_->mFreq;
}

void xAudioBuffer::UnMap(int data_size)
{
	X_ASSERT(data_size < this_->mBufferSize);
	this_->mDataSize = data_size;
}