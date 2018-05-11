#include "StdAfx.h"
#include "xTrack.h"
#include "xEdit.h"
using std::max;

boost::shared_ptr<IEdit>		CreateQVodEdit()
{
	boost::shared_ptr<IEdit> pEdit(new xEdit);
	return boost::dynamic_pointer_cast<IEdit>(pEdit);
}

class xEdit::impl
{
public:
	impl()
		: mStatus(STATUS_STOP)
	{
		mVideoTracks.push_back(ITrackPtr(new xTrack(TT_VIDEO)));
		mAudioTracks.push_back(ITrackPtr(new xTrack(TT_AUDIO)));
	}

public:
	std::deque< ITrackPtr>   mVideoTracks;
	std::deque< ITrackPtr>   mAudioTracks;
	qvodedit_mutex			 mMutex;

	EDIT_STATUS				mStatus;
};

xEdit::xEdit(void)
: this_(new impl())
{

}

xEdit::~xEdit(void)
{

}

ITrackPtr xEdit::AddTrack(TRACK_TYPE t)
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	if (t == TT_VIDEO)
	{
		this_->mVideoTracks.push_back(ITrackPtr(new xTrack(TT_VIDEO)));
		return this_->mVideoTracks.back();
	}
	else
	{
		this_->mAudioTracks.push_back(ITrackPtr(new xTrack(TT_AUDIO)));
		return this_->mAudioTracks.back();
	}
}

bool xEdit::DelTrack(ITrackPtr p)
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	if (p->GetTrackType() == TT_VIDEO)
	{
		this_->mVideoTracks.erase(std::find(this_->mVideoTracks.begin(), this_->mVideoTracks.end(), p));
	}
	else
	{
		this_->mAudioTracks.erase(std::find(this_->mAudioTracks.begin(), this_->mAudioTracks.end(), p));
	}
	return true;
}

ITrackPtr xEdit::GetTrack(TRACK_TYPE t, int track_id)
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	return t == TT_VIDEO ? this_->mVideoTracks[track_id] : this_->mAudioTracks[track_id];
}

int xEdit::GetTrackCount(TRACK_TYPE t)
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	return t == TT_VIDEO ? (int)this_->mVideoTracks.size() : (int)this_->mAudioTracks.size();
}

int xEdit::GetTrackID(ITrackPtr p)
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	if (p->GetTrackType() == TT_VIDEO)
	{
		return (int)std::distance(this_->mVideoTracks.begin(), std::find(this_->mVideoTracks.begin(), this_->mVideoTracks.end(), p));
	}
	else
	{
		return (int)std::distance(this_->mAudioTracks.begin(), std::find(this_->mAudioTracks.begin(), this_->mAudioTracks.end(), p));
	}
}

XREFTIME xEdit::GetDuration()
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	XREFTIME tmp = 0;
	BOOST_FOREACH(ITrackPtr& p, this_->mVideoTracks)
	{
		IClipPtr clip = p->GetLastClip();
		if (clip)
		{
			tmp = max(tmp,  clip->GetEndPos());
		}
	}

	BOOST_FOREACH(ITrackPtr& p, this_->mAudioTracks)
	{
		IClipPtr clip = p->GetLastClip();
		if (clip)
		{
			tmp = max(tmp,  clip->GetEndPos());
		}
	}
	return tmp;
}

IEdit*	xEdit::GetIEdit()
{
	return (IEdit*)this;
}

void xEdit::SetStatus(EDIT_STATUS s)
{

}

void xEdit::GetVideoStreamInfo(VIDEO_STREAM_INFO& video_stream_info)
{

}

void xEdit::GetAudioStreamInfo(AUDIO_STREAM_INFO& audio_stream_info)
{

}

EDIT_STATUS xEdit::GetStatus()
{
	qvodedit_mutex::scoped_lock lock(this_->mMutex);
	return this_->mStatus;
}

void xEdit::Seek(XREFTIME pos)
{

}

xObjectPtr		xEdit::GetVideoResultBuffer()
{
	return xObjectPtr();
}

void			xEdit::FreeVideoResultBuffer(xObjectPtr)
{

}

xObjectPtr		xEdit::GetAudioResultBuffer()
{
	return xObjectPtr();
}

void			xEdit::FreeAudioResultBuffer(xObjectPtr)
{

}