#pragma once

#include <set>
#include <deque>
#include <vector>
#include <map>
#include <guiddef.h>
#include <rpc.h>
#include <atlstr.h>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <windows.h>

#include <QtGui>
#include <QtCore>
#include <QtGlobal>

#include "TranscodeStruct.h"
#include "ITranscode.h"

#include "xTypeDefines.h"

#define MAX_SHARED_MEMORY	4096
#define WM_CMDLINE_ACTIVE	WM_USER+1001
//#define WM_LANG_CHANGE		WM_USER+1002

#pragma warning( disable :  4251 )  
#pragma warning( disable :  4099 )