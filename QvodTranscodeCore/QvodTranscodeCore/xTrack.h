#pragma once
// ***************************************************************
// Desc: 时间线上编辑轨道封装
//
// Copyright (c) 2011, QVOD Corporation.  All rights reserved.
// ***************************************************************
// 
// ***************************************************************
#include "xObject.h"
#include "IEdit.h"

class xTrack : public xObject, public ITrack
{
public:
	xTrack(TRACK_TYPE);
	~xTrack(void);

//ITrack impl
public:
	virtual TRACK_TYPE	GetTrackType()	const;
	virtual IClipPtr	AddClip(CLIP_TYPE, const wchar_t* url);
	virtual bool		DeleteClip( IClipPtr clip );

	virtual IClipPtr	GetPriorClip( IClipPtr pClip);
	virtual IClipPtr	GetNextClip( IClipPtr pClip );
	virtual IClipPtr	GetLastClip();
	virtual int			GetClipsFromTimeRange( XREFTIME fromPos,XREFTIME toPos, std::vector<IClipPtr>& retClips );

public:
	IClipPtr			GetClipAtTmPos( XREFTIME tPos );
	//IClipPtr			GetClipByGUID(const GUID& id) const;
	
protected:
#pragma warning( disable :  4251 )
	class impl;
	boost::shared_ptr<impl> this_;
#pragma warning( default :  4251 )
};
typedef boost::shared_ptr<xTrack> xTrackPtr;
typedef boost::weak_ptr<xTrack>	  xTrackPtr_;