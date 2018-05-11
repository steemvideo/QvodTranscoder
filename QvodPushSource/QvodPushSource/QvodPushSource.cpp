#include "PushSource.h"

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{ 
	&MEDIATYPE_NULL     // clsMajorType
	, &MEDIASUBTYPE_NULL  // clsMinorType
};


const AMOVIESETUP_PIN sudOpPin =
{
	L"Output"          // strName
	, FALSE              // bRendered
	, TRUE               // bOutput
	, FALSE              // bZero
	, FALSE              // bMany
	, &CLSID_NULL        // clsConnectsToFilter
	, L"Input"           // strConnectsToPin
	, 1                  // nTypes
	, &sudOpPinTypes     // lpTypes
};

const AMOVIESETUP_FILTER QvodPushSource =
{ 
	&CLSID_PUSH_SOURCE       // clsID
	, L"Qvod Push Source"  // strName
	, MERIT_UNLIKELY         // dwMerit
	, 1                      // nPins
	, &sudOpPin              // lpPin
};


//
//  Object creation template
//
CFactoryTemplate g_Templates[] = {
	{ 
		L"Qvod Push Source"
			, &CLSID_PUSH_SOURCE
			, CPushSource::CreateInstance
			, NULL
			, &QvodPushSource 
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
