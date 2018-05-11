#pragma once

#ifdef QVODTRANSCODECORE_EXPORTS
#define API_EXPORT	__declspec(dllexport)
#else
#define API_EXPORT	__declspec(dllimport)
#endif


E_CONTAINER API_EXPORT	ContainerFromString(CString str);
E_V_CODEC	API_EXPORT	VideoEncoderFromString(CString str);
E_A_CODEC	API_EXPORT	AudioEncoderFromString(CString str);
BOOL		API_EXPORT	GetMediaInfo(const wchar_t* url, boost::shared_ptr<Qvod_DetailMediaInfo>& pMideinfo);
void		API_EXPORT  Dec2(const BYTE* pSrc,BYTE* pDes,int len,unsigned int key2);
