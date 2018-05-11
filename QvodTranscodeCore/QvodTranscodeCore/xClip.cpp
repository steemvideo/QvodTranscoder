#include "StdAfx.h"
#include "IEdit.h"
#include "xClip.h"

class xClip::impl
{
public:
	impl(CLIP_TYPE t, const wchar_t* url)
		: mType(t)
		, mURL(url)
	{
		//UuidCreate(&mGUID);

		mTmLength = 120000000;
	}

	TM_PAIR							mEditRange;	//Clip的编辑范围(局部时间)
	XREFTIME						mTmPos;		//clip的时间线放置位置
	XREFTIME						mTmLength;
	boost::weak_ptr<ITrack>			mpContainer;//所属Track

	//GUID							mGUID;
	const wchar_t*							mURL;
	const wchar_t*							mName;
	CLIP_TYPE						mType;
};

xClip::xClip(CLIP_TYPE t, const wchar_t* url)
: this_(new impl(t, url))
{

}

xClip::~xClip(void)
{
}

void xClip::SetPos(XREFTIME tmPos)
{
	this_->mTmPos = tmPos;
}

XREFTIME xClip::GetPos() const
{
	return this_->mTmPos;
}

TM_PAIR xClip::GetOnlineTimePair() const
{
	TM_PAIR _pair(this_->mTmPos, GetEndPos());	
	return _pair;
}

TM_PAIR xClip::GetEditRange() const
{
	return this_->mEditRange;
}

XREFTIME xClip::GetEndPos() const
{
	return this_->mTmPos +this_-> mTmLength;
} 

// GUID&  xClip::GetGUID()
// {
// 	return this_->mGUID;
// }

void xClip::SetClipContainer(boost::weak_ptr<ITrack> t)
{
	this_->mpContainer = t;
}

const wchar_t* xClip::GetName() const
{
	return this_->mName;
}

void xClip::SetName(const wchar_t* str)
{
	this_->mName = str;
}
