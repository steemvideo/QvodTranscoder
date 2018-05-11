#pragma once
#include "windows.h"
class CReg
{
protected:
	CReg(void);
public:
	~CReg(void);
	static CReg* GetInstance();
	void OpenHDAcc(const wchar_t *szKey, const wchar_t *szValue);
	bool IsNoHDAcc(const wchar_t *szKey, const wchar_t *szValue);
	bool CoreAVCReg();
	bool ItemExist(HKEY hKEY, const wchar_t *pitemPath);
	//Destop play
	bool RegOpen(const wchar_t *szKey, HKEY &hKEY);
	bool CreateDestopPlay(const wchar_t *szKey);
	bool SetDestopPlay(HKEY hKEY, const wchar_t *pPicturePath, DWORD nSysColor, DWORD nPlayflag);
	bool RegValueQuery(HKEY hKEY, const wchar_t *szKey, const wchar_t *szValue,  BYTE* pValue, DWORD &len);
	bool RegValueSet(HKEY hKEY, const wchar_t *szKey, const wchar_t *pKeyValue, DWORD regtype,  BYTE* pValue, DWORD &len);
	bool RegCreate(HKEY hKEY, const wchar_t *szKey, const wchar_t *pKeyValue, DWORD regtype);
	//bool RegQueryValue(const wchar_t *szKey, const wchar_t *szValue, DWORD *pValue);
	bool RegAudioUnLock(HKEY hKEY, const wchar_t *szKey, const wchar_t *pexeName);
protected:
	static CReg m_Reg;
	static int m_nRef;
};
