#include "stdafx.h"
#include "Coredump.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <Tlhelp32.h>
#include <wchar.h>
#pragma comment(lib,"dbghelp")
#pragma comment(lib,"shlwapi.lib")
#include <stdio.h>


#define DUMPNAME _T("crash.dmp")
#define LOGNAME _T("crash.log")
//#define _DEBUG_COREDUMP

TCHAR CoreDump::m_processVersion[20] ={0};
TCHAR CoreDump::m_processName[60] = {0};
TCHAR CoreDump::m_reportExePath[MAX_PATH] = {0};
BOOL CoreDump::m_bStartHardAcc = FALSE;

BOOL CoreDump::SetExceptionHandle(CString processName,
								  CString processVersion,
								  CString reportExePath)
{
	if ( NULL == SetUnhandledExceptionFilter(CoreDump::MyUnhandledExceptionFilter) )
	{
		return FALSE;
	}
	lstrcpyn(m_processName,processName,MAX_PATH);
	lstrcpyn(m_processVersion,processVersion,MAX_PATH);
	lstrcpyn(m_reportExePath,reportExePath,MAX_PATH);
	return TRUE;
}
void CoreDump::SetHardAccStatus(BOOL bStart)
{
	m_bStartHardAcc = bStart;
}
LONG WINAPI CoreDump::MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	TCHAR szTempDir[MAX_PATH] = {0};
	TCHAR szDumpFile[MAX_PATH] = {0};
	TCHAR szLogFile[MAX_PATH] = {0};
	GetTempDirPath(szTempDir);
	//OutputDebugString(szTempDir);

	_sntprintf_s(szDumpFile,MAX_PATH,_T("%s\\%s"),szTempDir,DUMPNAME);

	if(!WriteDump(ExceptionInfo,szDumpFile))
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}
	_sntprintf_s(szLogFile,MAX_PATH,_T("%s\\%s"),szTempDir,LOGNAME);
	//WriteLog(szLogFile);

	HMODULE hMoudle = NULL;
	TCHAR szModuleName[MAX_PATH] = {0};
	if(!GetExceptionInfo(ExceptionInfo->ExceptionRecord->ExceptionAddress,
		hMoudle,szModuleName,ARRAYSIZE(szModuleName))
		)
	{
		lstrcpyn(szModuleName,m_processName,MAX_PATH);
	}
	DWORD offsetAddr = (DWORD)ExceptionInfo->ExceptionRecord->ExceptionAddress - (DWORD)(void*)hMoudle;
	TCHAR szPara[MAX_PATH*5]={0};
	_sntprintf_s(szPara,MAX_PATH*5, _T("-d %s -f %s  -pn %s -pv %s -mn %s -addr 0x%x -s"),
		         szTempDir,DUMPNAME,m_processName, m_processVersion, szModuleName,(DWORD)offsetAddr);
	//OutputDebugString(szPara);
	RunRexport(szPara);
	return EXCEPTION_EXECUTE_HANDLER;
}
void CoreDump::SuspendOthersThread()
{
}
BOOL CoreDump::GetExceptionInfo(IN PVOID pExceptionAddr,OUT HMODULE &hModule,OUT LPTSTR lpMouduleName, UINT nSize)
{
	if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pExceptionAddr, &hModule))
	{
		TCHAR szModuleName[MAX_PATH]={0};
		if(GetModuleFileName(hModule, szModuleName, nSize) > 0)
		{
			TCHAR *p = _tcsrchr(szModuleName,_T('\\'));
			if(p)
			{
				lstrcpyn(lpMouduleName,p+1,nSize);
			}
			else
			{
				lstrcpyn(lpMouduleName,szModuleName,nSize);
			}
			p = _tcsrchr(lpMouduleName,_T('.'));
			if(p)
			{
				*p = 0;
			}
			return TRUE;
		}
	}
	return FALSE;
}
void CoreDump::GetTempDirPath(OUT LPTSTR lpTempDirPath)
{
	TCHAR szSysTmp[MAX_PATH] = {0};
	TCHAR szTmpDir[MAX_PATH] = {0};
	GetTempPath(MAX_PATH,szSysTmp);
	_sntprintf_s(szTmpDir,MAX_PATH,_T("%s~QC%llx.tmp"),szSysTmp,time(0));
	lstrcpyn(lpTempDirPath,szTmpDir,MAX_PATH);
	if(PathFileExists(lpTempDirPath))		 
	{
		DeleteFile(lpTempDirPath);	
	}
	CreateDirectory(lpTempDirPath,NULL);
}
BOOL CoreDump::WriteDump(struct _EXCEPTION_POINTERS* ExceptionInfo,LPCTSTR lpDumpPath)
{
	HANDLE hDumpFile = CreateFile(lpDumpPath,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	//OutputDebugString(L"000000000000000000\n");
	//OutputDebugString(lpDumpPath);
	if ( hDumpFile == INVALID_HANDLE_VALUE )
	{
		//OutputDebugString(L"1111111111111111111\n");
		return FALSE;
	}
	BOOL bRet = FALSE;
	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	bRet = MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hDumpFile, MiniDumpNormal,&loExceptionInfo,NULL,NULL);
	CloseHandle(hDumpFile);
	return bRet;
}

void CoreDump::RunRexport(LPCTSTR lpPara)
{
	//MessageBox(0,lpPara,0,0);
#ifdef _DEBUG_COREDUMP
	::ShellExecute(NULL ,_T("open") , m_reportExePath , lpPara , NULL ,SW_SHOWNORMAL);
#else
	//OutputDebugString(L"33333333333333333\n");
	//OutputDebugString(m_reportExePath);
	//OutputDebugString(lpPara);
	::ShellExecute(NULL ,_T("open") , m_reportExePath , lpPara , NULL ,0);
#endif
}
