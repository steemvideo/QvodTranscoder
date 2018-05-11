#include "stdafx.h"
#include "Reg.h"
#include "stdio.h"
#include <wchar.h>
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

CReg CReg::m_Reg;
int CReg::m_nRef(0);
CReg::CReg(void)
{
}

CReg::~CReg(void)
{
}

CReg* CReg::GetInstance()
{
	return &m_Reg;
}

void CReg::OpenHDAcc(const wchar_t *szKey, const wchar_t *szValue)
{
	 HKEY hKey = NULL;
	 RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ|KEY_WRITE, &hKey);
	 if(hKey)
	 {
		 DWORD dwType, dwcbData = 4;
		 DWORD ret;
		 if(RegQueryValueEx(hKey, szValue, 0, &dwType, (byte*)&ret, &dwcbData) == ERROR_SUCCESS)
		 {
			 if(ret)
			 {
				 ret = 0;
				 RegSetValueEx(hKey, szValue, 0, REG_DWORD, (byte*)&ret, 4);
			 }
		 }
		 RegCloseKey(hKey);
	 }
}

bool CReg::IsNoHDAcc(const wchar_t *szKey, const wchar_t *szValue)
{
	bool bBaned = FALSE;
	HKEY hKey = NULL;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ|KEY_WRITE, &hKey);
	if(hKey)
	{
		DWORD dwType, dwcbData = 4;
		DWORD ret;
		if(RegQueryValueEx(hKey, szValue, 0, &dwType, (byte*)&ret, &dwcbData) == ERROR_SUCCESS)
		{
			if(ret)
			{
				bBaned = TRUE;
			}
		}
		RegCloseKey(hKey);
	}
	return bBaned;
}

bool CReg::CoreAVCReg()
{
	
	DWORD dwRet;
	//打开注册表项  [HKEY_LOCAL_MACHINE\SOFTWARE\CoreCodec\CoreAVC Pro]
	LPCWSTR wstrRegPath=L"SOFTWARE\\CoreCodec\\CoreAVC Pro";
	HKEY hKEY;
	long ret=::RegCreateKeyEx(
		HKEY_LOCAL_MACHINE
		,wstrRegPath
		,0
		,NULL
		,REG_OPTION_VOLATILE
		,KEY_ALL_ACCESS
		,NULL
		,&hKEY
		,&dwRet);

	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}
	//写注册表
	ret= RegSetValueEx(hKEY,L"User",NULL,REG_SZ,(const BYTE*)L"Registered User",(DWORD)(wcslen(L"Registered User")*2));
	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}

	ret= RegSetValueEx(hKEY,L"Serial",NULL,REG_SZ,(const BYTE*)L"IQIKB-6F7KD-CORE-IXRJW-IGUHC"
		,(DWORD)(wcslen(L"IQIKB-6F7KD-CORE-IXRJW-IGUHC")*2));
	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}
	::RegCloseKey(hKEY);

	//打开注册表项  [HKEY_LOCAL_MACHINE\SOFTWARE\CoreCodec\CoreAVC Pro 2.x]
	LPCWSTR wstrRegPath2=L"SOFTWARE\\CoreCodec\\CoreAVC Pro 2.x";
	ret=::RegCreateKeyEx(
		HKEY_LOCAL_MACHINE
		,wstrRegPath2
		,0
		,NULL
		,REG_OPTION_VOLATILE
		,KEY_ALL_ACCESS
		,NULL
		,&hKEY
		,&dwRet);

	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}
	//写注册表
	ret= RegSetValueEx(hKEY,L"User",NULL,REG_SZ,(const BYTE*)L"ryomikai33@gundam.eu",(DWORD)(wcslen(L"ryomikai33@gundam.eu") * 2));
	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}

	ret= RegSetValueEx(hKEY,L"Serial",NULL,REG_SZ,(const BYTE*)L"3TDVPV-XCNFDT-VAZVCD-17JVPC-16DCLL"
		,(DWORD)(wcslen(L"3TDVPV-XCNFDT-VAZVCD-17JVPC-16DCLL")*2));
	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}

//打开注册表项  [HKEY_LOCAL_MACHINE\SOFTWARE\CoreCodec\CoreAVC Pro 3.x]
	LPCWSTR wstrRegPath3=L"SOFTWARE\\CoreCodec\\CoreAVC Pro 3.x";
	ret=::RegCreateKeyEx(
		HKEY_LOCAL_MACHINE
		,wstrRegPath3
		,0
		,NULL
		,REG_OPTION_VOLATILE
		,KEY_ALL_ACCESS
		,NULL
		,&hKEY
		,&dwRet);

	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}
	//写注册表
	ret= RegSetValueEx(hKEY,L"User",NULL,REG_SZ,(const BYTE*)L"rbogaar@aol.com",(DWORD)(wcslen(L"rbogaar@aol.com") * 2));
	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}

	ret= RegSetValueEx(hKEY,L"Serial",NULL,REG_SZ,(const BYTE*)L"TBCYS8-Q18CKZ-8FHGG8-F49LTA-UK8AQQ"
		,(DWORD)(wcslen(L"TBCYS8-Q18CKZ-8FHGG8-F49LTA-UK8AQQ")*2));
	if(ERROR_SUCCESS != ret)
	{
		::RegCloseKey(hKEY);
		return false;
	}
	::RegCloseKey(hKEY);

	return true;
}

bool CReg::RegOpen(const wchar_t *szKey, HKEY &hKEY)
{
	DWORD dwRet;
	long ret = RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ|KEY_WRITE, &hKEY);
	if(ret == ERROR_SUCCESS)
	{
		return true;
	}
	else
	{
		ret = ::RegCreateKeyEx(
				HKEY_CURRENT_USER
				,szKey
				,0
				,NULL
				,REG_OPTION_VOLATILE
				,KEY_ALL_ACCESS
				,NULL
				,&hKEY
				,&dwRet);
		if(ret == ERROR_SUCCESS)
		{
			return true;
		}
	}
	return false;
}

bool CReg::CreateDestopPlay(const wchar_t *szKey)
{
	if(szKey == NULL)
	{
		return false;
	}
	HKEY hKEY;
	DWORD dwRet;
	wchar_t str[100];
	memset((BYTE*)&str, 0, 100 * sizeof(wchar_t));
	wcscpy(str, szKey);
	wcscat(str, L"\\DestopPicture");
	long ret=::RegCreateKeyEx(
		HKEY_CURRENT_USER
		,str
		,0
		,NULL
		,REG_OPTION_VOLATILE
		,KEY_ALL_ACCESS
		,NULL
		,&hKEY
		,&dwRet);
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	wcscpy(str, szKey);
	wcscat(str, L"\\SysColor");
	ret=::RegCreateKeyEx(
		HKEY_CURRENT_USER
		,str
		,0
		,NULL
		,REG_OPTION_VOLATILE
		,KEY_ALL_ACCESS
		,NULL
		,&hKEY
		,&dwRet);
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	wcscpy(str, szKey);
	wcscat(str, L"\\RecoverFlag");
    ret=::RegCreateKeyEx(
		HKEY_CURRENT_USER
		,str
		,0
		,NULL
		,REG_OPTION_VOLATILE
		,KEY_ALL_ACCESS
		,NULL
		,&hKEY
		,&dwRet);
		if(ret != ERROR_SUCCESS)
	{
		return false;
	}
		return true;
}

bool CReg::SetDestopPlay(HKEY hKEY, const wchar_t *pPicturePath, DWORD nSysColor, DWORD nPlayflag)
{
	DWORD ret;
	if(pPicturePath  != NULL)
	{
		ret = RegSetValueEx(hKEY,L"DestopPicture",NULL,REG_SZ,(const BYTE*)pPicturePath
			,(DWORD)(wcslen(pPicturePath)*2));
	}
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	ret= RegSetValueEx(hKEY,L"SysColor",NULL,REG_DWORD,(const BYTE*)&nSysColor
		,(DWORD)sizeof(nSysColor));
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	ret= RegSetValueEx(hKEY,L"RecoverFlag",NULL,REG_DWORD,(const BYTE*)&nPlayflag
		,(DWORD)sizeof(nPlayflag));
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	return true;
}

bool CReg::RegValueQuery(HKEY hKEY, const wchar_t *szKey,  const wchar_t *szValue,  BYTE* pValue, DWORD &len)
{
	if(szKey == NULL || szValue == NULL || pValue == NULL)
	{
		return false;
	}
	HKEY hKey;
	bool ret = false;
	DWORD re = 0;
	re = RegOpenKeyEx(hKEY, szKey, 0, KEY_READ|KEY_WRITE, &hKey);
	if(re != ERROR_SUCCESS)
	{
		return false;
	}
	DWORD regType = 0;
	if(RegQueryValueEx(hKey, szValue, 0,&regType, pValue, &len) == ERROR_SUCCESS)
	{
		ret = true;
	}
	::RegCloseKey(hKey);
	return ret;
}

bool CReg::RegValueSet(HKEY hKEY, const wchar_t *szKey, const wchar_t *pKeyValue, DWORD regtype,  BYTE* pValue, DWORD &len)
{
	HKEY hkey;
	DWORD ret = RegOpenKeyEx(hKEY, szKey, 0, KEY_READ|KEY_WRITE, &hkey);
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	ret= RegSetValueEx(hkey, pKeyValue, NULL, regtype, pValue, len);
	::RegCloseKey(hkey);
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	return true;
}

bool CReg::RegCreate(HKEY hKEY, const wchar_t *szKey, const wchar_t *pKeyValue, DWORD regtype)
{
	HKEY hkey = NULL;
	DWORD ret=::RegCreateKeyEx(hKEY
								,szKey
								,0
								,NULL
								,REG_OPTION_VOLATILE
								,KEY_ALL_ACCESS
								,NULL
								,&hkey
								,&regtype);
	if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	else
	{
		RegCloseKey(hkey);
	}
	return true;
}

bool CReg::RegAudioUnLock(HKEY hKEY, const wchar_t *szKey, const wchar_t *pexeName)
{
	if(szKey == NULL || pexeName == NULL)
	{
		return false;
	}
	HKEY hkey;
	 DWORD ret  =  RegOpenKeyEx( hKEY,
				   szKey,
					0,
					KEY_READ,
					&hkey);
    if(ret != ERROR_SUCCESS)
	{
		return false;
	}
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
 
    DWORD i, retCode; 
 
    //TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
 
    // 获取注册表项的子项信息
    retCode = RegQueryInfoKey(
        hkey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
	if (retCode != ERROR_SUCCESS) 
    {
		RegCloseKey(hKEY);
        return false;
    }
 
    // 应用程序音频输出关联信息
	BYTE Value3[16] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00};
	BYTE Value4[24] = {0x04, 0x20,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F};
	BYTE Value5[16] = {0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    TCHAR path[MAX_PATH];
	TCHAR path2[MAX_PATH];
	memset(path, 0, MAX_PATH * sizeof(TCHAR));
	wcscpy(path, szKey);
	wcscat(path, L"\\%s\\{219ED5A0-9CBF-4F3A-B927-37C9E5C5F14F}");
	int len = wcslen(path);
    if (cSubKeys)
    {
        for (i=0; i<cSubKeys; i++) //逐项检查各个音频输出关联项
        { 
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hkey, i,
                     achKey, 
                     &cbName, 
                     NULL, 
                     NULL, 
                     NULL, 
                     &ftLastWriteTime); 
            if (retCode != ERROR_SUCCESS) 
            {
				RegCloseKey(hKEY);
                return false;
            }
			HKEY hsubKey;
			bool ret = false;
			//DWORD re = 0;
			memset(path2, 0, sizeof(TCHAR)*MAX_PATH);
			wcscpy(path2, szKey);
			int len2 = wcslen(path2);
			path2[len2] = '\\';
			wcscat(path2, achKey);
			retCode = RegOpenKeyEx(hKEY, path2, 0, KEY_READ|KEY_WRITE, &hsubKey);
			if(retCode != ERROR_SUCCESS)
			{
				continue;
			}
			DWORD regType = 0;
			DWORD len = MAX_VALUE_NAME;
			TCHAR NameValue[MAX_VALUE_NAME];
			if(RegQueryValueEx(hsubKey, NULL, 0,&regType, (LPBYTE)NameValue, &len) == ERROR_SUCCESS)
			{
				if(regType == REG_SZ && wcsstr(NameValue, pexeName) != NULL)
				{
					BYTE value[24];
					bool isdelete = false;
					memset(value, 0, sizeof(TCHAR));
					swprintf(path2, path, achKey);
					len = 24;
					if(RegValueQuery(hKEY, path2,  L"3", value, len) && memcmp(value, Value3, 16) != 0)
					{
						len = 16;
					    RegValueSet(hKEY, path2, L"3", REG_BINARY,  Value3, len);
					    isdelete = true;
					}
					len = 24;
					if(RegValueQuery(hKEY, path2,  L"4", value, len) && memcmp(value, Value4, 24) != 0)
					{
						len = 24;
					    RegValueSet(hKEY, path2, L"4", REG_BINARY,  Value4, len);
					    isdelete = true;
					}
					len = 24;
					if(RegValueQuery(hKEY, path2,  L"5", value, len) && memcmp(value, Value4, 24) != 0)
					{
					    len = 16;
					    RegValueSet(hKEY, path2, L"5", REG_BINARY,  Value5, len);
					    isdelete = true;
					}
				}
			}
			::RegCloseKey(hsubKey);
        }
    }
	RegCloseKey(hkey);
	return true;
}

//bool CReg::RegQueryValue(const wchar_t *szKey, const wchar_t *szValue, DWORD *pValue)
//{
//	if(szKey == NULL || szValue == NULL)
//	{
//		return false;
//	}
//	HKEY hKey;
//	bool ret = false;
//	DWORD re = 0;
//	re = RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ|KEY_WRITE, &hKey);
//	if(re != ERROR_SUCCESS)
//	{
//		return false;
//	}
//	DWORD dwcbData = 4;
//	if(RegQueryValueEx(hKey, szValue, 0, REG_DWORD, (byte*)&pValue, &dwcbData) == ERROR_SUCCESS)
//	{
//		ret = true;
//	}
//	::RegCloseKey(hKey);
//	return ret;
//}