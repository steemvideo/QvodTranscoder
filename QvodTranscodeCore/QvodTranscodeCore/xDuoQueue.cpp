#include "StdAfx.h"
#include "xDuoQueue.h"
#include "xUtility.h"
#include <boost/lambda/bind.hpp>
#include <boost/thread/condition.hpp>

enum 
{
	FREE_QUEUE   = 0,
	RESULT_QUEUE = 1 
};

class xDuoQueue::impl
{
public:
	impl()
		:mbFreequeContinue(true)
		,mbResultQueContinue(true)
	{};
	boost::mutex             mFreeMutex_,mResultMutex_;
	boost::condition         mCondFreeQueNotEmpty;
	boost::condition         mCondResultQueNotEmpty;
	std::deque<xObjectPtr>   mFreeQueue;
	std::deque<xObjectPtr>   mResultQueue;    
	std::deque<xObjectPtr>   mTotalContainer;    
	size_t                   mMaxQueSize; 
	bool					 mbFreequeContinue;
	bool					 mbResultQueContinue;
};

xDuoQueue::xDuoQueue()
:this_( new impl)
{

}

bool xDuoQueue::Create( const std::deque< xObjectPtr >& ptr_elems )
{
	{
		boost::mutex::scoped_lock _fLock( this_->mFreeMutex_   );
		this_->mFreeQueue = ptr_elems;  
	}
	{
		boost::mutex::scoped_lock _RLock( this_->mResultMutex_ );
		this_->mResultQueue.clear();            
	} 
	this_->mMaxQueSize = ptr_elems.size();
	return true;
}

void xDuoQueue::PushToResultQueue  ( xObjectPtr pObj )
{
	if( NULL == pObj )   return;
	boost::mutex::scoped_lock _RLock( this_->mResultMutex_ );    
	{
		this_->mResultQueue.push_back( pObj );
		this_->mCondResultQueNotEmpty.notify_one();
		X_ASSERT( this_->mResultQueue.size() <= this_->mMaxQueSize );
	}    

}

void xDuoQueue::PushToFreeQueue( xObjectPtr pObj )
{
	if( NULL == pObj )   return;
	boost::mutex::scoped_lock _FLock( this_->mFreeMutex_ );    
	{
		this_->mFreeQueue.push_back( pObj );
		this_->mCondFreeQueNotEmpty.notify_one();
		X_ASSERT( this_->mFreeQueue.size() <= this_->mMaxQueSize );
	}    
}

xObjectPtr xDuoQueue::PopFromResultQueue ()
{
	boost::mutex::scoped_lock _lock( this_->mResultMutex_ );
	if (this_->mbResultQueContinue)
	{
		if( this_->mResultQueue.empty() )
		{          
			this_->mCondResultQueNotEmpty.wait(_lock);    
		}
		if( this_->mResultQueue.empty() ) 
			return xObjectPtr();
		xObjectPtr pObj  = this_->mResultQueue.front();
		this_->mResultQueue.pop_front();
		return pObj;
	}   
	return xObjectPtr();
}

xObjectPtr xDuoQueue::PopFromFreeQueue()
{
	boost::mutex::scoped_lock _lock( this_->mFreeMutex_ );
	if (this_->mbFreequeContinue)
	{
		if(  this_->mFreeQueue.empty() )
		{          
			this_->mCondFreeQueNotEmpty.wait(_lock);    
		}

		if( this_->mFreeQueue.empty() ) 
			return xObjectPtr();

		xObjectPtr pObj  = this_->mFreeQueue.front();
		this_->mFreeQueue.pop_front();
		return pObj;
	}   
	return xObjectPtr();
}

void xDuoQueue::OnRescue(void)
{
	{
		boost::mutex::scoped_lock lock(this_->mResultMutex_);
		this_->mCondResultQueNotEmpty.notify_all();
	}
	{
		boost::mutex::scoped_lock lock(this_->mFreeMutex_);
		this_->mCondFreeQueNotEmpty.notify_all();
	}
}

void xDuoQueue::onDestory()
{
	{
		boost::mutex::scoped_lock lock(this_->mResultMutex_);
		this_->mbResultQueContinue = false;
		this_->mCondResultQueNotEmpty.notify_all();
	}
	{
		boost::mutex::scoped_lock lock(this_->mFreeMutex_);
		this_->mbFreequeContinue = false;
		this_->mCondFreeQueNotEmpty.notify_all();
	}
}

void xDuoQueue::Flush()
{
	boost::mutex::scoped_lock _lock_1( this_->mFreeMutex_   );
	boost::mutex::scoped_lock _lock_2( this_->mResultMutex_ );
	if( !this_->mResultQueue.empty() )
	{
		for(int i=0;i<(int)this_->mResultQueue.size();i++ )
		{
			xObjectPtr _mobj = this_->mResultQueue[i];           
			this_->mFreeQueue.push_back( _mobj );
		}
		this_->mResultQueue.clear();
	}
	this_->mCondFreeQueNotEmpty.notify_all();
	this_->mCondResultQueNotEmpty.notify_all();
}

int xDuoQueue::GetResultBufferCount(void)
{
	boost::mutex::scoped_lock _lock_2( this_->mResultMutex_ );
	return (int)this_->mResultQueue.size();
}