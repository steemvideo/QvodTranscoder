#include "stdafx.h"
#include "MediaInfo.h"
#include "xUtility.h"
#include "streams.h"
#include <exception>
#include "windows.h"
//using namespace System;

CMediaInfo CMediaInfo::m_MediaInfo;
CMediaInfo::CMediaInfo(void)
{
	m_hMediaInfo_dll = NULL;
	m_Get_FileInfo = NULL;
	m_Get_StreamInfo = NULL;
	m_Get_DetailInfo = NULL;
	m_Get_FileLabel = NULL;
}

CMediaInfo::~CMediaInfo(void)
{
	if(m_hMediaInfo_dll)
	{
		FreeLibrary(m_hMediaInfo_dll);
	}
	m_hMediaInfo_dll = NULL;
}

CMediaInfo* CMediaInfo::GetInstance()
{
	return &m_MediaInfo;
}
BOOL CMediaInfo::init()
{
	if(m_hMediaInfo_dll == NULL)
	{
		//if(pFileUrl)
		{
			//wchar_t dllpath[MAX_PATH];
			//::GetModuleFileName((HMODULE)g_hInstance, dllpath, MAX_PATH);
			TCHAR current_path[512];
			if(GetQvodPlayerPath(current_path))
			{
				CString url = current_path;
				//int tmp = dp.ReverseFind('\\') + 1;
				//CString url = dp.Left(tmp);
				url += L"\\QMediaInfo.dll";
				m_hMediaInfo_dll = LoadLibrary(url.GetBuffer(url.GetLength() + 1));
			}
			
		}

		if(m_hMediaInfo_dll == NULL)
		{
			m_hMediaInfo_dll = LoadLibrary(L"QMediaInfo.dll");
		}
		if(m_hMediaInfo_dll)
		{
			m_Get_FileInfo = (GETFILEINFO)GetProcAddress(m_hMediaInfo_dll,"GetFileMediaInfo");
			m_Get_StreamInfo = (GETSTREAMINFO)GetProcAddress(m_hMediaInfo_dll,"GetStreamMediaInfo");
			m_Get_DetailInfo = (GETFILEDETAILINFO)GetProcAddress(m_hMediaInfo_dll,"GetFileDetailMediaInfo");
			m_Get_FileLabel = (GETFILELABEL)GetProcAddress(m_hMediaInfo_dll,"GetFileLabel");
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}
BOOL CMediaInfo::GetMediaMainInfo(const wchar_t* strURL,Qvod_MainMediaInfo *pMainInfo)
{
	CheckPointer(strURL, FALSE);
	CheckPointer(pMainInfo, FALSE);

	if(m_hMediaInfo_dll == NULL)
	{
		init();
	}

	if(m_Get_FileInfo)
	{
		return m_Get_FileInfo(strURL, pMainInfo);
	}
	return FALSE;
}
BOOL CMediaInfo::GetStreamInfo(IAsyncReader *pReader,Qvod_MainMediaInfo *pMainInfo)
{
	CheckPointer(pReader, FALSE);
	CheckPointer(pMainInfo, FALSE);

	if(m_hMediaInfo_dll == NULL)
	{
		init();
	}

	//try
	//{
		if(m_Get_StreamInfo)
		{
			return m_Get_StreamInfo(pReader, pMainInfo);
		}
	/*}
	catch(std::exception& e)
	{*/
		//MessageBox(NULL, NULL, NULL, MB_TOPMOST);
	//}
	return FALSE;
}
BOOL CMediaInfo::GetDetailMediaInfo(const wchar_t* strURL,boost::shared_ptr<Qvod_DetailMediaInfo>& pDetailInfo)
{
	CheckPointer(strURL, FALSE);
	CheckPointer(pDetailInfo, FALSE);

	if(m_hMediaInfo_dll == NULL)
	{
		init();
	}


	if(m_Get_DetailInfo)
	{
		return m_Get_DetailInfo(strURL, pDetailInfo.get());
	}
	//}
	//catch(std::exception& e)
	//{
	//	const char* str = e.what();
	//	OutputDebugStringA(str);
	//	MessageBoxA(NULL, "System Error!\n", e.what(), MB_TOPMOST);
	//}
	
	return FALSE;
}
BOOL CMediaInfo::GetFileInfo(const wchar_t* strURL,s_FileInfo *pFileInfo)
{
	CheckPointer(strURL, FALSE);
	CheckPointer(pFileInfo, FALSE);

	if(m_hMediaInfo_dll == NULL)
	{
		init();
	}

	if(m_Get_FileLabel)
	{
		return m_Get_FileLabel(strURL, pFileInfo);
	}
	return FALSE;
}