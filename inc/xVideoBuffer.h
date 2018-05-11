#pragma once
#include "xObject.h"
#include "TranscodeStruct.h"

class   xVideoBuffer : public xObject
{
public:
	xVideoBuffer( EVIDEO_CHROMA_FMT fmt, int pic_width, int pic_height);

	int		GetPixWidth() const;
	int		GetPixHeight() const;

	void*	Map( ECOLOR_CHANNEL _eChannel, int& widt_in_byte, int& height, int& width_pitch );
protected:
	int mWidth;
	int mHeight;
	int mPitch;
	EVIDEO_CHROMA_FMT mChromaFmt;

	boost::shared_array<byte>	mpData[4];//[0]Y [1]U [2]V [3]A
};

typedef boost::shared_ptr<xVideoBuffer> xVideoBufferPtr;
typedef boost::weak_ptr<xVideoBuffer>	xVideoBufferPtr_;
