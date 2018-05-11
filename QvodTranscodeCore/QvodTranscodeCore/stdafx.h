#pragma once



#include <set>
#include <deque>
#include <vector>
#include <map>
#include <algorithm>
#include <guiddef.h>
#include <rpc.h>
//#include <Strmif.h>
#include <atlstr.h>
#include "MediaInfo.h"

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/win32/recursive_mutex.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/foreach.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/random_access_index.hpp>


#include <windows.h>
#include "Strmif.h"
#include "mtype.h"
#include "uuids.h"
#include "streams.h"
// thread mutex
typedef boost::recursive_mutex      qvodedit_mutex;


#pragma warning( disable :  4251 )  