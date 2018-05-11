#pragma once
// ***************************************************************
// Desc: 时间线上编辑单位封装
//
// Copyright (c) 2011, QVOD Corporation.  All rights reserved.
// ***************************************************************
// 
// ***************************************************************
#include "xobject.h"
#include "TranscodeStruct.h"

#include "IEdit.h"

class xClip : public xObject, public IClip
{
	friend class xTrack;
public:
	xClip(CLIP_TYPE, const wchar_t*);
	~xClip(void);
	

//IClip
public:
	TM_PAIR     GetOnlineTimePair()		const;
	TM_PAIR		GetEditRange()			const;
	const wchar_t*		GetName() const;
	void		SetName(const wchar_t*);

	XREFTIME	GetPos()				const;
	XREFTIME	GetEndPos()				const;
	//GUID&		GetGUID();

public:
	void		SetPos(XREFTIME tmPos);
	void		SetClipContainer(boost::weak_ptr<ITrack>);
protected:
	class impl;
	boost::shared_ptr<impl>	this_;
};

typedef boost::shared_ptr<xClip>		xClipPtr;
typedef boost::weak_ptr<xClip>			xClipPtr_;