#include "stdafx.h"

  bool operator <(const GUID& elem0,const GUID& elem1 )
{
	return memcmp( &elem0, &elem1, sizeof(GUID) )< 0;
}

  bool GetQvodPlayerPath(TCHAR* path)  
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
	  // ¶ÁÈ¡¼üÖµÄÚÈÝ    
	  DWORD type = REG_SZ;   
	  DWORD dwInfoSize = 512;   
	  ret = RegQueryValueEx(hKEY, _T("Insertpath"), NULL, &type, (BYTE*)path, &dwInfoSize);   
	  if(ret!=ERROR_SUCCESS)  
	  {   
		  RegCloseKey(hKEY);   
		  return false;   
	  }  
	  RegCloseKey(hKEY);   
	  return true;  
  }  
