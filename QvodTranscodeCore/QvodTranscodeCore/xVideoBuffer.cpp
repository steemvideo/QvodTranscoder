#include "StdAfx.h"
#include "xVideoBuffer.h"

xVideoBuffer::xVideoBuffer( EVIDEO_CHROMA_FMT fmt, int width, int height)
: mWidth(width)
, mChromaFmt(fmt)
, mHeight(height)
{
	switch(fmt)
	{
	case	CHROMA_I420:
	case	CHROMA_YV12:
		mpData[0].reset(new byte[width*height]);
		mpData[1].reset(new byte[width*height/4]);
		mpData[2].reset(new byte[width*height/4]);
		break;
	case	CHROMA_I422:
		mpData[0].reset(new byte[width*height]);
		mpData[1].reset(new byte[width*height/2]);
		mpData[2].reset(new byte[width*height/2]);
		break;
	case	CHROMA_I444:
	case	CHROMA_444RGB:
		mpData[0].reset(new byte[width*height]);
		mpData[1].reset(new byte[width*height]);
		mpData[2].reset(new byte[width*height]);
		break;
	case	CHROMA_444RGBA:
		mpData[0].reset(new byte[width*height]);
		mpData[1].reset(new byte[width*height]);
		mpData[2].reset(new byte[width*height]);
		mpData[3].reset(new byte[width*height]);

		break;
	default:
		break;
	}
	
}

void* xVideoBuffer::Map( ECOLOR_CHANNEL _eChannel, int& width_in_bytes, int& height, int& pitch )
{
	void * pRet = NULL;
	width_in_bytes = 0;
	height = 0;
	pitch = 0;

	int	 BytesPerPix = 1;

	switch(mChromaFmt)
	{
	case	CHROMA_I420:
		{
			height = mHeight;
			width_in_bytes = mWidth*BytesPerPix;
			if(_eChannel == CHANNEL_Y)
			{
				pitch = width_in_bytes;
				pRet = (void*)mpData[0].get();
			}
			else if (_eChannel == CHANNEL_U)
			{
				pitch = width_in_bytes / 4;
				pRet = (void*)mpData[1].get();
			}
			else if(_eChannel == CHANNEL_V)
			{
				pitch = width_in_bytes / 4;
				pRet = (void*)mpData[2].get();
			}
		}
		break;
	case	CHROMA_YV12:
		{
			height = mHeight;
			width_in_bytes = mWidth*BytesPerPix;
			if(_eChannel == CHANNEL_Y)
			{
				pitch = width_in_bytes;
				pRet = (void*)mpData[0].get();
			}
			else if (_eChannel == CHANNEL_U)
			{
				pitch = width_in_bytes / 4;
				pRet = (void*)mpData[2].get();
			}
			else if(_eChannel == CHANNEL_V)
			{
				pitch = width_in_bytes / 4;
				pRet = (void*)mpData[1].get();
			}
		}
		break;
	case	CHROMA_I422:
		{
			height = mHeight;
			width_in_bytes = mWidth*BytesPerPix;
			if(_eChannel == CHANNEL_Y)
			{
				pitch = width_in_bytes;
				pRet = (void*)mpData[0].get();
			}
			else if (_eChannel == CHANNEL_U)
			{
				pitch = width_in_bytes/2;
				pRet = (void*)mpData[1].get();
			}
			else if(_eChannel == CHANNEL_V)
			{
				pitch = width_in_bytes/2;
				pRet = (void*)mpData[2].get();
			}
		}
		break;
	case	CHROMA_I444:
	case	CHROMA_444RGB:
		{
			height = mHeight;
			width_in_bytes = mWidth*BytesPerPix;
			pitch = width_in_bytes;
			if(_eChannel == CHANNEL_Y)
			{
				pRet = (void*)mpData[0].get();
			}
			else if (_eChannel == CHANNEL_U)
			{
				pRet = (void*)mpData[1].get();
			}
			else if(_eChannel == CHANNEL_V)
			{
				pRet = (void*)mpData[2].get();
			}
		}
		break;
	case	CHROMA_444RGBA:
		{
			height = mHeight;
			width_in_bytes = mWidth*BytesPerPix;
			pitch = width_in_bytes;
			if(_eChannel == CHANNEL_Y || _eChannel == CHANNEL_R)
			{
				pRet = (void*)mpData[0].get();
			}
			else if (_eChannel == CHANNEL_U  || _eChannel == CHANNEL_G)
			{
				pRet = (void*)mpData[1].get();
			}
			else if(_eChannel == CHANNEL_V  || _eChannel == CHANNEL_B)
			{
				pRet = (void*)mpData[2].get();
			}
			else
			{
				pRet = (void*)mpData[3].get();
			}
		}
		break;
	default:
		break;
	}
	return pRet;
}

int xVideoBuffer::GetPixWidth() const
{
	return mWidth;
}
int	xVideoBuffer::GetPixHeight() const
{
	return mHeight;
}
