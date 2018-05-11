#pragma once
#include "TranscodeStruct.h"
class ITrack;
class IClip
{
public:
	virtual TM_PAIR     GetOnlineTimePair()	const	= 0;
	virtual TM_PAIR		GetEditRange()		const	= 0;
	virtual const wchar_t*		GetName()			const	= 0;
	virtual void		SetName(const wchar_t*)			= 0;
	//virtual void		GetMixClips(std::vector<boost::shared_ptr<IClip> >& clips) = 0; 
	
	virtual XREFTIME	GetPos()				const = 0;
	virtual XREFTIME	GetEndPos()				const = 0;
	virtual void		SetPos(XREFTIME)			= 0;
	virtual void		SetClipContainer(boost::weak_ptr<ITrack>) = 0;

	void*				GetHandle() const { return (void*)this; }
};
typedef boost::shared_ptr<IClip>	IClipPtr;
typedef boost::weak_ptr<IClip>		IClipPtr_;

class ITrack
{
public:
	virtual TRACK_TYPE	GetTrackType()	const					= 0;
	virtual IClipPtr	AddClip(CLIP_TYPE t, const wchar_t* url)		= 0;
	virtual bool		DeleteClip( IClipPtr clip )				= 0;

	virtual IClipPtr	GetPriorClip( IClipPtr pClip)			= 0;
	virtual IClipPtr	GetNextClip( IClipPtr pClip )			= 0;
	virtual IClipPtr	GetLastClip()							= 0;
	virtual	int			GetClipsFromTimeRange( XREFTIME fromPos,XREFTIME toPos, std::vector<IClipPtr>& retClips) = 0;
};
typedef boost::shared_ptr<ITrack>	ITrackPtr;
typedef boost::weak_ptr<ITrack>		ITrackPtr_;

class IEdit
{
public:
	virtual ITrackPtr		AddTrack(TRACK_TYPE)				= 0;
	virtual bool			DelTrack(ITrackPtr)					= 0;

	virtual ITrackPtr		GetTrack(TRACK_TYPE, int track_id)	= 0;
	virtual int				GetTrackCount(TRACK_TYPE)			= 0;
	virtual int				GetTrackID(ITrackPtr)				= 0;
	virtual	XREFTIME		GetDuration()						= 0;
};
typedef boost::shared_ptr<IEdit>  IEditPtr;
typedef boost::weak_ptr<IEdit>	  IEditPtr_;