#include "stdafx.h"
#include "xMainGraph.h"
#include "IPushSource.h"
#include "xEdit.h"
#include "xUtility.h"
#include "dmodshow.h"
#include "commonfunction.h"
#include "Reg.h"

extern CString CLSID2String(const CLSID& clsid);
extern void	CStringtoBin(CString str, BYTE *pdata);

xMainGraph::xMainGraph(IDataSourcePtr datasource)
: mDataSource(datasource)
{
	init();
}

xMainGraph::xMainGraph()
{
	const GUID CLSID_COREAVC = {0x09571A4B, 0xF1FE, 0x4C60, {0x97, 0x60, 0xDE, 0x6D, 0x31, 0x0C, 0x7C, 0x31}};
	QVOD_FILTER_INFO filterinfo;
	mpCoreAVC = LoadFilter(CLSID_COREAVC, filterinfo);
	init();
}

xMainGraph::~xMainGraph(void)
{
	if (mpGraph)
	{
		DestroyGraph();
	}
}

void xMainGraph::init()
{
	// New IGraphBuilder
	mpGraph = LoadFilterGraph();
	mdwRegister = 0;

#ifdef SHOW_GRAPH_IN_GRAPHEDIT
	if (mpGraph)
		AddToRot((IUnknown *)mpGraph, &mdwRegister);
#endif
}

void xMainGraph::SetPlayWnd(HWND hwnd)
{
	mhPlayWnd = hwnd;
}

CComPtr<IGraphBuilder> xMainGraph::BuildGraph()
{
	// Push Source Filter
	mPushSourceFilter = LoadSouceFilter();
	if (!mPushSourceFilter)
	{
		return CComPtr<IGraphBuilder>(); 
	}
	// Configure Push Source
	if (SUCCEEDED(mPushSourceFilter->QueryInterface(IID_IPushSource, (void**)&mIPushSource)))
	{
		// set IDataSource to PushSource Filter

		
	}

	// Load Video Multi Tee
	mVideoMultiTeeFilter = LoadMultiOutputFilter();	
	// Connect IPushSource && Multi Tee
	if (!ConnectFilers(mPushSourceFilter, mVideoMultiTeeFilter))
	{
		return CComPtr<IGraphBuilder>();
	}

	// Load Video Render
	LoadVideoRender();
	// Connect IPushSource && Multi Tee
	if (!ConnectFilers(mVideoMultiTeeFilter, mpFilterVideoRender))
	{
		return CComPtr<IGraphBuilder>();
	}

	// Load Video Multi Tee
	mAudioMultiTeeFilter = LoadMultiOutputFilter();	
	// Connect IPushSource && Multi Tee
	if (!ConnectFilers(mPushSourceFilter, mAudioMultiTeeFilter))
	{
		return CComPtr<IGraphBuilder>();
	}
	// Load Audio Render
	CComPtr<IBaseFilter>	audio_render = LoadAudioRender();
	// Connect IPushSource && Multi Tee
	if (!ConnectFilers(mAudioMultiTeeFilter, audio_render))
	{
		return CComPtr<IGraphBuilder>();
	}

	return mpGraph;
}

CComPtr<IGraphBuilder> xMainGraph::LoadFilterGraph()
{
	HRESULT hr = E_FAIL;

	//IGraphBuilding
	CComPtr<IGraphBuilder> pGraph;
	HMODULE hmd = LoadLibrary(L"Quartz.dll");
	if(hmd == NULL)
	{
		return CComPtr<IGraphBuilder>();
	}

	pfnDllGetClassObject fnDllGetClassObject = (pfnDllGetClassObject) GetProcAddress(hmd,	"DllGetClassObject");
	if(!fnDllGetClassObject)
	{
		::FreeLibrary(hmd);
		hmd = NULL;
		return CComPtr<IGraphBuilder>();
	}

	if(fnDllGetClassObject)
	{//find class id
		CComPtr<IClassFactory> pClassFactory;
		hr = fnDllGetClassObject(CLSID_FilterGraph, IID_IClassFactory, (void **)&pClassFactory);
		if(SUCCEEDED(hr) && pClassFactory)
		{
			hr = pClassFactory->CreateInstance(NULL, IID_IFilterGraph, (void **)&pGraph);	
		}
	}

	::FreeLibrary(hmd);
	if (FAILED(hr))
	{
		pGraph.Release();
	}
	return pGraph;
}

#ifdef SHOW_GRAPH_IN_GRAPHEDIT
HRESULT xMainGraph::AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
	CComPtr<IMoniker> pMoniker;
	CComPtr<IRunningObjectTable> pROT;
	if(FAILED(GetRunningObjectTable(0, &pROT)))
	{
		return E_FAIL;
	}

	WCHAR wsz[256];
	wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
	HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if(SUCCEEDED(hr))
	{
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,	pMoniker, pdwRegister);
	}

	return hr;
	//return S_OK;
}

void xMainGraph::RemoveFromRot(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;
	if(SUCCEEDED(GetRunningObjectTable(0, &pROT)))
	{
		HRESULT hr = pROT->Revoke(pdwRegister);
		X_ASSERT(hr == S_OK);
		pROT->Release();
	}
}
#endif

// CComPtr<IBaseFilter> xMainGraph::LoadFilter(const CLSID& clsid, QVOD_FILTER_INFO& filter_info)
// {
// 
// }

void xMainGraph::Trace(int loglevel, const wchar_t* pFormat, ...)
{
		if(loglevel <= ENABLE_LOG_LEVEL)
		{
			va_list _Arglist;
			int _Ret, _Count = 0;
			wchar_t info[MAX_PATH];
			memset(info, 0, MAX_PATH * sizeof(wchar_t));

			va_start(_Arglist, pFormat);
			_Ret = _vswprintf_c_l(info, MAX_PATH, pFormat, NULL, _Arglist);
			va_end(_Arglist);
#ifdef ENABLE_DEBUG_INFO
			OutputDebugStringW(info);
#endif
		}
}

CComPtr<IBaseFilter> xMainGraph::LoadSouceFilter()
{
	CComPtr<IBaseFilter>	pbasefilter;
	HMODULE hmodule = LoadLibrary(L"QvodPushSource.dll");
	if(!hmodule)
	{	
		return CComPtr<IBaseFilter>();
	}

	// 创建IBaseFilter接口
	pfnDllGetClassObject	fnDllGetClassObject = (pfnDllGetClassObject) GetProcAddress(hmodule,	"DllGetClassObject");
	if(!fnDllGetClassObject)
	{
		::FreeLibrary(hmodule);
	}

	// Create IBaseFilter
	std::vector<HMODULE> modules;
	CComPtr<IClassFactory> pClassFactory;
	HRESULT hr = fnDllGetClassObject(CLSID_PUSH_SOURCE,IID_IClassFactory, (void **)&pClassFactory);
	if(SUCCEEDED(hr) && pClassFactory)
	{
		hr = pClassFactory->CreateInstance(NULL, IID_IBaseFilter, (void**)(&pbasefilter));
		if(SUCCEEDED(hr) && pbasefilter)
		{
			modules.push_back(hmodule);
		}
	}

	if (pbasefilter)
	{
		if(!AddFiltertoGraph(pbasefilter, L"Qvod PushSource Filter"))
		{
			pbasefilter.Release();
			return CComPtr<IBaseFilter>();
		}
		else
		{
			// 记录加载的组件
			mModule[pbasefilter] = modules;
		}
	}
	return pbasefilter;
}

CComPtr<IBaseFilter>	xMainGraph::LoadMultiOutputFilter()
{
	// load infinite Pin Tee Filter
	const GUID guid_tee = {0xF8388A40,0xD5BB,0x11D0,{0xBE,0x5A,0x00,0x80,0xC7,0x06,0x56,0x8E}};
	CComPtr<IBaseFilter>	pbasefilter;
	HMODULE hmodule = LoadLibrary(L"qcab.dll");
	if(!hmodule)
	{	
		return CComPtr<IBaseFilter>();
	}

	// 创建IBaseFilter接口
	pfnDllGetClassObject	fnDllGetClassObject = (pfnDllGetClassObject) GetProcAddress(hmodule,	"DllGetClassObject");
	if(!fnDllGetClassObject)
	{
		::FreeLibrary(hmodule);
	}

	// Create IBaseFilter
	std::vector<HMODULE> modules;
	CComPtr<IClassFactory> pClassFactory;
	HRESULT hr = fnDllGetClassObject(guid_tee,IID_IClassFactory, (void **)&pClassFactory);
	if(SUCCEEDED(hr) && pClassFactory)
	{
		hr = pClassFactory->CreateInstance(NULL, IID_IBaseFilter, (void**)(&pbasefilter));
		if(SUCCEEDED(hr) && pbasefilter)
		{
			modules.push_back(hmodule);
		}
	}

	if (pbasefilter)
	{
		if(!AddFiltertoGraph(pbasefilter, L"Infinite Pin Tee Filter"))
		{
			pbasefilter.Release();
			return CComPtr<IBaseFilter>();
		}
		else
		{
			// 记录加载的组件
			mModule[pbasefilter] = modules;
		}
	}
	return pbasefilter;
}

bool xMainGraph::AddFiltertoGraph(CComPtr<IBaseFilter>& pbasefilter,  LPCWSTR pName)
{
	if(!pbasefilter || !mpGraph)
		return false;

	HRESULT hr = mpGraph->AddFilter(pbasefilter, pName);

	return SUCCEEDED(hr);
}

bool xMainGraph::ConnectFilers(CComPtr<IBaseFilter>& pSrc, CComPtr<IBaseFilter>& pDest)
{
	// 获取Dest Filter的输入媒体类型
	if (!pSrc || !pDest)
	{
		return false;
	}

	// 循环每个输出PIN
	bool ret = false;
	BeginEnumPins(pSrc, pEnumPins, pOutPin)
	{	
		PIN_DIRECTION out_direction;
		HRESULT hr = pOutPin->QueryDirection(&out_direction);
		if(SUCCEEDED(hr) && out_direction == PINDIR_OUTPUT)
		{
			// 循环输出PIN的媒体类型
			BeginEnumMediaTypes(pOutPin, pEnumMediaTypes, pOutMediaType)
			{
				// 找到输入PIN
				bool bFind = false;
				BeginEnumPins(pDest, pDecoderEnumPins, pInPin)
				{
					bFind = true;
					// 找到
					if (SUCCEEDED(pInPin->QueryAccept(pOutMediaType)))
					{
						CComPtr<IPin> pConnectto;
						pInPin->ConnectedTo(&pConnectto);
						if(pConnectto)
						{
							pConnectto.Release();
							continue;
						}
						// 连接两个PIN
						hr =  mpGraph->ConnectDirect(pOutPin,pInPin,pOutMediaType);
						if(SUCCEEDED(hr))
						{
							ret = true;
						}
					}
				}
				EndEnumPins
				X_ASSERT(bFind);	
			}
			EndEnumMediaTypes(pOutMediaType)
		}
	}
	EndEnumPins
	return ret;//false;
}

bool xMainGraph::LoadVideoRender()
{
	QVOD_FILTER_INFO info;
	mpFilterVideoRender = LoadFilter(CLSID_VideoRenderer, info);
	if(!mpFilterVideoRender)
	{
		return false;
	}

	if (mpFilterVideoRender)
	{
		CComPtr<IVMRFilterConfig> pConfig;
		HRESULT hRes2 = mpFilterVideoRender->QueryInterface(IID_IVMRFilterConfig, (LPVOID *)&pConfig);
		if(SUCCEEDED(hRes2))
		{
			hRes2 = pConfig->SetRenderingMode(VMRMode_Windowless);
		}
		hRes2 = mpFilterVideoRender->QueryInterface(IID_IVMRWindowlessControl, (LPVOID *)&mpVMRWindowless);
		if(SUCCEEDED(hRes2))
		{
			mpVMRWindowless->SetVideoClippingWindow(mhPlayWnd);

			RECT rect;
			GetClientRect(mhPlayWnd, &rect);
			mpVMRWindowless->SetVideoPosition(NULL, &rect);
		}
	}

	CComPtr<IBaseFilter> pSrc = mVideoMultiTeeFilter?mVideoMultiTeeFilter:mPushSourceFilter;
	if(FAILED(ConnectFilers(pSrc, mpFilterVideoRender)))
	{
		return false;
	}
	return true;
}

bool xMainGraph::CreateAudioRender()
{
// 	if(mConType == _E_CON_MIDI)
// 	{
// 		HRESULT hr = CoCreateInstance(CLSID_AVIMIDIRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&mpFilterAudioRender);
// 		if(SUCCEEDED(hr))
// 		{
// 			mSoundDevice = L"MIDI Render";
// 			IAMResourceControl* pIAMResourceControl= NULL;
// 			hr = mpFilterAudioRender->QueryInterface(IID_IAMResourceControl, (void**)&pIAMResourceControl);
// 			X_ASSERT(hr == S_OK && pIAMResourceControl);
// 			hr = pIAMResourceControl->Reserve(AMRESCTL_RESERVEFLAGS_RESERVE, NULL);
// 			X_ASSERT(hr == S_OK);
// 			SafeRelease(pIAMResourceControl);
// 		}
// 	}
// 	else
// 	{
		long audioDevice = 0;
		if (!audioDevice)
		{
			HRESULT hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&mpFilterAudioRender);
			if(FAILED(hr))
			{
				mpFilterAudioRender.Release();
			}
			mSoundDevice = L"Default DirectSound Device";
		}
		else
		{
			// 遍历，找出
			ICreateDevEnum *pSysDevEnum = NULL;
			HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
			if (FAILED(hr))
			{
				return NULL;
			}

			IEnumMoniker *pEnumMoniker = NULL;
			hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumMoniker, 0);
			if(FAILED(hr))
			{	
				SafeRelease(pSysDevEnum);
				return NULL;
			}
			pEnumMoniker->Reset();
			IMoniker *pMoniker = NULL;
			long norder = 0;
			for (pEnumMoniker; pEnumMoniker->Next(1, &pMoniker, 0) == S_OK;)
			{
				LPOLESTR olestr = NULL;
				if(FAILED(pMoniker->GetDisplayName(0, 0, &olestr)))
				{
					SafeRelease(pMoniker);
					continue;
				}

				IPropertyBag *pPB = NULL;
				if(SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPB)))
				{
					CComVariant var;
					GUID gaudio;
					CString fstr;
					if(SUCCEEDED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, NULL)))
					{
						mSoundDevice  = var.bstrVal;
						var.Clear();
					}
					if(SUCCEEDED(pPB->Read(CComBSTR(_T("CLSID")), &var, NULL)))
					{
						CLSIDFromString(var.bstrVal, &gaudio);
						var.Clear();
					}
					if(audioDevice == 0 && CLSID_DSoundRender == gaudio)
					{//default
						pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&mpFilterAudioRender);
						break;				
					}
					if(norder == audioDevice-1)
					{
						pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&mpFilterAudioRender);
						break;	
					}
					SafeRelease(pPB);
				}
				norder++;
				SafeRelease(pMoniker);
			}
			SafeRelease(pEnumMoniker);
			SafeRelease(pSysDevEnum);
		}
	//}

	if (mpFilterAudioRender)
	{
		if(!AddFiltertoGraph(mpFilterAudioRender,mSoundDevice.c_str()))
		{
			return false;
		}
	}

	return mpFilterAudioRender?true:false;
}



bool xMainGraph::ConnectAudioRenderAuto()
{
	if (!mpFilterAudioRender)
	{
		if(!CreateAudioRender())
		{
			return false;
		}
	}

	if (!mAudioMultiTeeFilter)
	{
		ReleaseFilter(mpFilterAudioRender);
		mpFilterAudioRender.Release();
		return false;
	}

	if (FAILED(ConnectFilterDirect(mAudioMultiTeeFilter, mpFilterAudioRender)))
	{
		ReleaseFilter(mpFilterAudioRender);
		mpFilterAudioRender.Release();
		return false;
	}

	return true;
}

HRESULT xMainGraph::ConnectFilterDirect(CComPtr<IBaseFilter>& pSrc, CComPtr<IBaseFilter>& pDest)
{
	// 尝试不指定类型连接
	BeginEnumPins(pSrc, pEnumPins, pOutPin)
	{
		PIN_DIRECTION thisPinDir;
		pOutPin->QueryDirection(&thisPinDir);
		if (thisPinDir == PINDIR_OUTPUT)
		{
			CComPtr<IPin> pTmp;
			pOutPin->ConnectedTo(&pTmp);
			if (!pTmp)
			{
				if(SUCCEEDED(ConnectPinToFilers(pOutPin, pDest)))
				{
					return S_OK;
				}
			}
		}
	}
	EndEnumPins
	return E_FAIL;
}

HRESULT xMainGraph::ConnectPinToFilers(CComPtr<IPin>& pOutPin,CComPtr<IBaseFilter>& pDest)
{
	if (pOutPin == NULL || pDest == NULL)
	{
		return E_POINTER;
	}

// 	if (mConType == _E_CON_DVD && pDest == mpFilterVideoRender)
// 	{
// 		HRESULT hr = E_FAIL;
// 		BeginEnumMediaTypes(pOutPin, pEnumMediaTypes, pOutMediaType)
// 		{
// 			// #4082 强制DVD不能硬件加速
// 			if (!mPlayMedia->m_pDXVAChecker->CheckDXVAGUID(&pOutMediaType->subtype, &pOutMediaType->formattype))
// 			{
// 				// 找到输入PIN
// 				BeginEnumPins(pDest, pDecoderEnumPins, pInPin)
// 				{
// 					// 找到
// 					if (SUCCEEDED(pInPin->QueryAccept(pOutMediaType)))
// 					{
// 						// 连接两个PIN
// 						hr =  mpGraph->ConnectDirect(pOutPin,pInPin, pOutMediaType);
// 						if(SUCCEEDED(hr))
// 						{
// 							if(pOutMediaType) 
// 								DeleteMediaType(pOutMediaType); 
// 							return S_OK;
// 						}
// 					}
// 				}
// 				EndEnumPins
// 			}
// 		}
// 		EndEnumMediaTypes(pOutMediaType)
// 			return hr;
// 	}
// 	else
	{
		CComPtr<IEnumPins> pEnum;
		CComPtr<IPin> pPin;
		HRESULT hr = pDest->EnumPins(&pEnum);
		if (FAILED(hr))
		{
			return hr;
		}

		while(pPin.Release(), pEnum->Next(1,&pPin, NULL) == S_OK)
		{
			PIN_DIRECTION thisPinDir;
			pPin->QueryDirection(&thisPinDir);
			if (thisPinDir == PINDIR_INPUT)
			{
				CComPtr<IPin> pTmp;
				hr = pPin->ConnectedTo(&pTmp);
				if (SUCCEEDED(hr))
				{
					continue;
				}
				else
				{
					hr = mpGraph->ConnectDirect(pOutPin,pPin, NULL);
					if (SUCCEEDED(hr))
					{
						return hr;
					}
				}
			}
			pPin.Release();
		}
		return E_FAIL;
	}
}

void xMainGraph::ReleaseFilter(CComPtr<IBaseFilter>& pbasefilter)
{
	if (pbasefilter)
	{
		IBaseFilter* filter = pbasefilter;
		HRESULT hr = mpGraph->RemoveFilter(pbasefilter);
		pbasefilter.Release();
		X_ASSERT(hr == S_OK);
	}
}

CComPtr<IBaseFilter> xMainGraph::LoadFilter(const CLSID& clsid, QVOD_FILTER_INFO& filter_info)
{
	CComPtr<IBaseFilter> pbasefilter;

	// 从XML中获取Filter
	if(!QvodFilterManager::GetInstance().GetFilterByCLSID(clsid, filter_info))
	{
		return CComPtr<IBaseFilter>();
	}

	// 平台系统检查
// 	if (filter_info.eSystem != e_Apply_All)
// 	{
// 		if (filter_info.eSystem == e_Apply_Vista_Only && version < 6)
// 			return CComPtr<IBaseFilter>();
// 		if (filter_info.eSystem == e_Apply_XP_Only && version >= 6)
// 			return CComPtr<IBaseFilter>();
// 	}

	// 设置路径等
	PrepareFilter(filter_info);

	// 装载依赖的组件并注册
	std::vector<HMODULE> modules;
	LoadComponents(filter_info, modules);

	// 装载DLL
	HMODULE hmodule = LoadLibrary(filter_info.path);
	if(hmodule == NULL)
	{
		hmodule = LoadModuleSpecy(filter_info.path);
	}
	if(!hmodule)
	{
		DWORD e= GetLastError();
		Trace(-1, L"dll 加载失败：%s, error code:%d\n",filter_info.path, e);
		return CComPtr<IBaseFilter>();
	}

	Trace(1, L"加载dll%s成功\n", filter_info.path);
	// 创建IBaseFilter接口
	pfnDllGetClassObject	fnDllGetClassObject = (pfnDllGetClassObject) GetProcAddress(hmodule,	"DllGetClassObject");
	if(!fnDllGetClassObject)
	{
		::FreeLibrary(hmodule);
	}
	else
	{
		modules.push_back(hmodule);
	}

	// Create IBaseFilter
	CLSID _clsid = filter_info.bDMO ? filter_info.wrapid : filter_info.clsid;
	CComPtr<IClassFactory> pClassFactory;
	HRESULT hr = fnDllGetClassObject(_clsid,IID_IClassFactory, (void **)&pClassFactory);
	if(SUCCEEDED(hr) && pClassFactory)
	{
		hr = pClassFactory->CreateInstance(NULL, IID_IBaseFilter, (void**)(&pbasefilter));
		if(SUCCEEDED(hr) && pbasefilter)
		{
			if (filter_info.bDMO)
			{
				CComPtr<IDMOWrapperFilter>   pDmoWrapper = NULL;   
				hr = pbasefilter->QueryInterface(IID_IDMOWrapperFilter,  reinterpret_cast<void**>(&pDmoWrapper));  
				if(SUCCEEDED(hr))     
				{
					hr = pDmoWrapper->Init(filter_info.clsid, filter_info.catid);
					if (FAILED(hr))
					{
						pbasefilter.Release();
						return CComPtr<IBaseFilter>();
					}
				} 
			}
		}
	}

	if (pbasefilter)
	{
		if(!AddFiltertoGraph(pbasefilter, filter_info.strName))
		{
			pbasefilter.Release();
			return CComPtr<IBaseFilter>();
		}
		else
		{
			// 记录加载的组件
			mModule[pbasefilter] = modules;
		}
	}

	return pbasefilter;
}

HMODULE	xMainGraph::LoadModuleSpecy(const CString  path)
{
	int index = path.ReverseFind(L'\\');
	CString newpath = path;
	if(index >= 0)
	{
		newpath = path.Right(path.GetLength() - index - 1);
	}

	if(newpath.Find(L"COREAVC.AX") >= 0)
	{
		CReg::GetInstance()->CoreAVCReg();
		return LoadLibrary(path);
	}

	return LoadLibrary(newpath);
}

void xMainGraph::PrepareFilter(QVOD_FILTER_INFO& filter_info)
{
	// 获取全路径
	RepalcePath(filter_info.path);
	for (int i = 0; i < filter_info.vComponents.size(); i++)
	{
		RepalcePath(filter_info.vComponents[i].path);
	}
}


void xMainGraph::RepalcePath(CString& path)
{
	if (path.IsEmpty())
	{
		return;
	}

	TCHAR output[MAX_PATH];
	memset(output, 0, MAX_PATH);

	path.MakeUpper();
	int pos = path.Find(L"%WINDOWS%");
	if(pos != -1)
	{		
		GetWindowsDirectory(output, MAX_PATH);
		wsprintf(output, _T("%s\\%s"), output, ((LPCTSTR)path)+10);
		path = output;
	}
	else
	{
		TCHAR current_path[512];
		//int len = GetCurrentDirectory(256, current_path);
		if(GetQvodPlayerPath(current_path))
		{
			wsprintf(output, _T("%s\\Codecs\\%s"), current_path,path);
			path = output;
		}
	}
}
/*
bool xMainGraph::GetQvodPlayerPath(TCHAR* path)  
{  
	HKEY hKEY;   
	//LPCTSTR Rgspath = _T("SoftWare\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\QvodPlayer");   
	LPCTSTR Rgspath = _T("SoftWare\\QvodPlayer\\Insert");   
	LONG ret = RegOpenKeyEx(HKEY_CURRENT_USER, Rgspath, 0, KEY_READ, &hKEY);   

	if(ret != ERROR_SUCCESS)   
	{   
		RegCloseKey(hKEY);   
		return false;   
	}   
	// 读取键值内容    
	DWORD type = REG_SZ;   
	DWORD dwInfoSize = 512;   
	ret = RegQueryValueEx(hKEY, _T("Insertpath"), NULL, &type, (BYTE*)path, &dwInfoSize);   
	if(ret!=ERROR_SUCCESS)  
	{   
		RegCloseKey(hKEY);   
		return false;   
	}  
	return true;  
}  
*/
void xMainGraph::DestroyGraph()
{
#ifdef SHOW_GRAPH_IN_GRAPHEDIT
	if(mdwRegister)
	{
		RemoveFromRot(mdwRegister);
		mdwRegister = 0;
	}

#endif

	// RemoveFilter及它接口
	mVideoMultiTeeFilter.Release();
	mAudioMultiTeeFilter.Release();
	mEdit.reset();
	mDataSource.reset();

	mpVMRWindowless.Release();

	Trace(2, L"before Release IGraphBuilder\n");
	// 释放IGraphBuilder接口
	int ret = 0;
	HRESULT hr = E_FAIL;
	if(mpGraph)
	{
		IEnumFilters *pEnum = NULL;
		IBaseFilter *pBF[20]= {0};
		ULONG fetched = 20;
		hr = mpGraph->EnumFilters(&pEnum);
		if(SUCCEEDED(hr) && pEnum)
		{
			pEnum->Next(20, pBF, &fetched);
			for(int i =0; i < fetched; i++)
			{
				hr = mpGraph->RemoveFilter(pBF[i]);
				ret = pBF[i]->Release();
			}
			pEnum->Release();
		} 

		mpGraph.Release();
	}
	Trace(2, L"after Release IGraphBuilder\n");
	// 释放dlls
	std::map<IBaseFilter*, std::vector<HMODULE> >::iterator it = mModule.begin();
	for (int j = 0; it != mModule.end(); it++, j++)
	{
		//if(j == 4)continue;
		for (int i = 0; i < it->second.size(); i++)
		{
			int size = 100;
			wchar_t str[100];
			swprintf(str, L"module:i:%d,j:%d ",i, j);
			int tip = wcslen(str);
			//OutputDebugString(str);
			GetModuleFileName(it->second[i], str, size);
			//OutputDebugString(str);
			::FreeLibrary(it->second[i]);
		}
	}
	mModule.clear();
	Trace(2, L"after Release dll\n");
}

bool xMainGraph::LoadComponents(QVOD_FILTER_INFO& filter_info, std::vector<HMODULE>&  modules)
{
	for (int i = 0; i < filter_info.vComponents.size(); i++)
	{
		if (!filter_info.vComponents[i].bReg)
		{
			HMODULE hmodule = ::LoadLibrary(filter_info.vComponents[i].path);
			if (hmodule)
			{
				modules.push_back(hmodule);
			}
			else
			{
				continue;
			}
		}
		else
		{
			CString lpSubKey = "CLSID\\";
			lpSubKey += (CLSID2String(filter_info.vComponents[i].clsid));

			const GUID CLSID_Haali_Source = {0x55DA30FC,0xF16B,0x49FC,{0xBA,0xA5,0xAE,0x59,0xFC,0x65,0xF8,0x2D}};
			const GUID CLSID_Haali_Splitter_AR = {0x564FD788,0x86C9,0x4444,{0x97,0x1E,0xCC,0x4A,0x24,0x3D,0xA1,0x50}};
			bool awalysreg = false;
			if (filter_info.vComponents[i].clsid == CLSID_Haali_Splitter_AR || filter_info.vComponents[i].clsid == CLSID_Haali_Source)
				awalysreg = true;

			// 检查是否注册且注册的文件是否存在
			HKEY key;
			wchar_t path[512];
			memset(path, 0, 512*sizeof(wchar_t));
			LONG len = 512;
			LONG ret = RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_READ, &key);
			if(ret ==ERROR_SUCCESS)
			{
				ret = RegQueryValue(key, _T("InprocServer32"), path, &len);
				if(ret == ERROR_SUCCESS && len > 0)
				{
					// 查看文件是否存在
					int ret = _waccess(path, 0);
					if(ret == 0 && !awalysreg)
					{
						RegCloseKey(key);
						continue;
					}
				}
				RegCloseKey(key);
			}

			// Load & Register
			HMODULE	hmodule = ::LoadLibrary(filter_info.vComponents[i].path);
			if (!hmodule)
			{
				continue;
			}

			modules.push_back(hmodule);

			typedef long (*pfnRegister)(void);
			pfnRegister pfnReg = NULL;

			pfnReg = (pfnRegister) GetProcAddress(hmodule, "DllRegisterServer");
			if(pfnReg)
			{
				HRESULT hr = pfnReg();
				if(FAILED(hr))
				{
					continue;
				}
			}
		}
	}
	return true;
}

bool xMainGraph::LoadAudioRender()
{
	return false;
}