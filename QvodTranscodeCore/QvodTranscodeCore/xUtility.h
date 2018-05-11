#pragma once


// Assert
#ifdef _DEBUG
#define X_ASSERT(p)  assert(p)
#else
#define X_ASSERT(p)
#endif

#define SafeDelete(pObject)		 if(pObject) {delete pObject; pObject = NULL;}
#define SafeDeleteArray(p)		 if(p){delete []p; p = NULL;}

#ifdef _WIN32
#define SafeRelease(pInterface)  if(pInterface) {pInterface->Release(); pInterface = NULL;}
#define SafeDelObj(pObject)		 if(pObject){::DeleteObject(pObject); pObject = 0;}	
#define SafeDelHandle(pHandle)	 if(pHandle){::CloseHandle(pHandle);pHandle = 0;}
#endif

// Debug Dump
#ifdef _DEBUG
#define X_TRACE1(x,p0)\
{\
	wchar_t szMsg[256];\
	swprintf(szMsg,256,L##x,p0);\
	OutputDebugStringW(szMsg);\
}
#define X_TRACE2(x,p0,p1)\
{\
	wchar_t szMsg[256];\
	swprintf(szMsg,256,L##x,p0,p1);\
	OutputDebugStringW(szMsg);\
}
#define X_TRACE3(x,p0,p1,p2)\
{\
	wchar_t szMsg[256];\
	swprintf(szMsg,256,L##x,p0,p1,p2);\
	OutputDebugStringW(szMsg);\
}
#define X_TRACE(p) OutputDebugStringW(L##p)
#else
#define X_TRACE3(x,p0,p1,p2)
#define X_TRACE2(x,p0,p1)
#define X_TRACE1(x,p0)
#define X_TRACE(p)
#endif



  bool operator <(const GUID& elem0,const GUID& elem1);
  bool GetQvodPlayerPath(TCHAR* path)  ;
