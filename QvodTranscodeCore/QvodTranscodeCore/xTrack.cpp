#include "StdAfx.h"
#include "TranscodeStruct.h"
#include "IEdit.h"
#include "xTrack.h"
#include "xUtility.h"
#include "xClip.h"

using namespace boost;
using namespace boost::multi_index;

INDEX_TAG_DEF( TAG_BY_TIME     )
INDEX_TAG_DEF( TAG_BY_CLIPUUID )
INDEX_TAG_DEF( TAG_BY_HANDLE   )

typedef  multi_index_container
<   IClipPtr ,
	indexed_by< 
	ordered_unique< tag<TAG_BY_TIME>     ,BOOST_MULTI_INDEX_CONST_MEM_FUN(IClip,TM_PAIR     ,GetOnlineTimePair)>,
	ordered_unique< tag<TAG_BY_HANDLE>   ,BOOST_MULTI_INDEX_CONST_MEM_FUN(IClip,void*     ,  GetHandle)>
	>
> _ClipsSet;
typedef _ClipsSet::index<TAG_BY_TIME>::type					ClipsSet_ByTime;
typedef _ClipsSet::index<TAG_BY_HANDLE>::type				ClipsSet_ByHandle;

class xTrack::impl
{	
public: 
	impl(TRACK_TYPE t)
		: mTrackType(t)
	{

	}
public:
	_ClipsSet                    mClips_Set;	 
	qvodedit_mutex               mMutex;
	TRACK_TYPE					 mTrackType;
	bool                         mbMute;
};

xTrack::xTrack(TRACK_TYPE t)
: this_(new impl(t))
{

}

xTrack::~xTrack(void)
{
}

TRACK_TYPE xTrack::GetTrackType() const
{
	return this_->mTrackType;
}

IClipPtr xTrack::AddClip(CLIP_TYPE t, const wchar_t* url)
{
	IClipPtr clip(new xClip(t, url));
	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
	ClipsSet_ByTime& curSet     = this_->mClips_Set.get<TAG_BY_TIME>();

	if(curSet.empty())
	{
		clip->SetPos(0);
	}
	else
	{
		IClipPtr clipOn = GetLastClip();
		if(clipOn)
			clip->SetPos(clipOn->GetEndPos());
		else
			clip->SetPos(0);
	}

	std::pair<ClipsSet_ByTime::iterator, bool> r = curSet.insert( clip );
	X_ASSERT(r.second);

	if(r.second)
	{
		xTrackPtr pCurTrack = boost::static_pointer_cast<xTrack>(getPtr());
		clip->SetClipContainer(pCurTrack );
		return clip;
	}
	else
	{
		return IClipPtr();
	}
	
}

 int xTrack::GetClipsFromTimeRange( XREFTIME fromPos,XREFTIME toPos, std::vector<IClipPtr>& retClips )
 {		
 	if (fromPos > toPos)
 		return 0;
 
 	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
 
 	TM_PAIR  from_tmPair( fromPos, fromPos),to_tmPair( toPos, toPos);	
 	ClipsSet_ByTime& tmSet     = this_->mClips_Set.get<TAG_BY_TIME>();
 	ClipsSet_ByTime::iterator it_start =  tmSet.lower_bound( from_tmPair );//(>=)
 	ClipsSet_ByTime::iterator it_end   =  tmSet.upper_bound( to_tmPair );//(>)
 #pragma warning( disable :  4244 )
 	int num = std::distance(it_start,it_end);
 #pragma warning( default :  4244 )
 	for( ClipsSet_ByTime::iterator it = it_start; it!= it_end ;++it )
 	{
 		retClips.push_back(*it);
 	}	
 	return num;	 
 } 
 
 IClipPtr xTrack::GetClipAtTmPos( XREFTIME tPos  )
 {
 	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
 	TM_PAIR  tmPair( tPos, tPos);	
 	ClipsSet_ByTime& tmSet         = this_->mClips_Set.get< TAG_BY_TIME >();
 	std::pair<ClipsSet_ByTime::iterator,ClipsSet_ByTime::iterator> _pair   = tmSet.equal_range(tmPair);
 	if( _pair.first != _pair.second ) 
 	{ 
 		return *_pair.first;
 	}
 	return IClipPtr();
 }
 
//  IClipPtr xTrack::GetClipByGUID(const GUID& id) const
//  {
//  	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
//  	ClipsSet_By_ClipUUID& tmSet          = this_->mClips_Set.get< TAG_BY_CLIPUUID >();
//  	ClipsSet_By_ClipUUID::iterator it    = tmSet.find( id );
//  	if( it != tmSet.end() )
//  	{           
//  		return *it;
//  	}
//  	return IClipPtr();
//  }
 
 IClipPtr xTrack::GetNextClip( IClipPtr pClip )
 {	
 	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
 	if( pClip )
 	{
		TM_PAIR TimePair                = TM_PAIR(pClip->GetEndPos() + 1, pClip->GetEndPos() + 1);
		ClipsSet_ByTime& tmSet          = this_->mClips_Set.get< TAG_BY_TIME >();
		ClipsSet_ByTime::iterator it    = tmSet.lower_bound(TimePair);
		if( it != tmSet.end() )
		{          
			return *it;
		}
 	}
 	return IClipPtr();
 }
 
 IClipPtr xTrack::GetPriorClip( IClipPtr pClip )
 {
 	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
 	if( pClip )
 	{
		TM_PAIR TimePair                    = TM_PAIR(pClip->GetPos() - 1, pClip->GetPos() - 1);
		ClipsSet_ByTime& tmSet              = this_->mClips_Set.get< TAG_BY_TIME >();
		ClipsSet_ByTime::iterator it        = tmSet.upper_bound( TimePair );
		if( it != tmSet.begin() && it != tmSet.end())
		{           	
			return *(--it);
		}
 	}
 	return IClipPtr();
 }
 
 IClipPtr xTrack::GetLastClip()
 {
	 qvodedit_mutex::scoped_lock lock(this_->mMutex); 
	 ClipsSet_ByTime& tmSet              = this_->mClips_Set.get< TAG_BY_TIME >();
	 int cout = tmSet.size();
	 ClipsSet_ByTime::reverse_iterator it        = tmSet.rbegin();
	 if (it != tmSet.rend())
	 {
		 return *it;
	 }
	 return IClipPtr();
 }

 bool xTrack::DeleteClip( IClipPtr pClip)
 { 	
 	qvodedit_mutex::scoped_lock lock(this_->mMutex); 
 	if( pClip)
 	{
 		ClipsSet_ByHandle& curSet      = this_->mClips_Set.get<TAG_BY_HANDLE>();
 		ClipsSet_ByHandle::iterator it = curSet.find( pClip.get());
 		if(it != curSet.end())
 		{
 			curSet.erase(it);
 			return true;
 		}
 	}
 	return false;
 }  
