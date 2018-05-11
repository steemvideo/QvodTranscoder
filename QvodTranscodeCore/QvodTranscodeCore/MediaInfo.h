#pragma once
#include <boost/shared_ptr.hpp>
#include "QMediaInfo.h"


extern HINSTANCE	g_hInstance;
class CMediaInfo
{
public:
	static CMediaInfo* GetInstance();
	BOOL init();
	BOOL GetMediaMainInfo(const wchar_t* strURL,Qvod_MainMediaInfo *pMainInfo);
	BOOL GetStreamInfo(IAsyncReader *pReader,Qvod_MainMediaInfo *pMainInfo);
	BOOL GetDetailMediaInfo(const wchar_t* strURL,boost::shared_ptr<Qvod_DetailMediaInfo>& pDetailInfo);
	BOOL GetFileInfo(const wchar_t* strURL,s_FileInfo *pLabelInfo);

public:
	~CMediaInfo(void);
protected:
		CMediaInfo(void);
		static CMediaInfo m_MediaInfo;
		HMODULE					m_hMediaInfo_dll;
		GETFILEINFO				m_Get_FileInfo;
		GETSTREAMINFO			m_Get_StreamInfo;
		GETFILEDETAILINFO		m_Get_DetailInfo;
		GETFILELABEL			m_Get_FileLabel;
};
