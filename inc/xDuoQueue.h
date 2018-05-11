#pragma once

#include "xObject.h"
class   xDuoQueue
{ 
public:
	xDuoQueue();
	bool	Create( const std::deque< boost::shared_ptr<xObject> >& ptr_elems );                       
	void	Flush();
	void	PushToResultQueue  ( xObjectPtr pObj );
	void	PushToFreeQueue    ( xObjectPtr pObj );

	xObjectPtr PopFromResultQueue ();
	xObjectPtr PopFromFreeQueue   ();

	void	OnRescue(void);
	void	onDestory();
	int		GetResultBufferCount(void);
protected:
	class impl;
	boost::shared_ptr<impl>  this_;	
};

typedef boost::shared_ptr<xDuoQueue>	xDuoQueuePtr;
typedef boost::weak_ptr<xDuoQueue>		xDuoQueuePtr_;