#ifndef _COREDUMP_H_H__
#define _COREDUMP_H_H__

#include <string>
#include <atlstr.h>
#include <tchar.h>
//#ifdef _DEBUG
#include <DbgHelp.h>
#include <time.h>

//#endif // _DEBUG
using std::string;

class CoreDump
{
public:
	static BOOL SetExceptionHandle(CString processName,
		                           CString processVersion,
								   CString reportExePath);
	static void SetHardAccStatus(BOOL bStart);
private:
	// 自定义异常过滤程序
	static LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo); 
	static void SuspendOthersThread();
	static BOOL GetExceptionInfo(IN PVOID pExceptionAddr,OUT HMODULE &hModule,OUT LPTSTR lpMouduleName, UINT nSize);
	static void GetTempDirPath(OUT LPTSTR lpTempDirPath);
	static BOOL WriteDump(struct _EXCEPTION_POINTERS* ExceptionInfo,LPCTSTR lpDumpPath);
	static void RunReportExe(LPCTSTR lpDumpPath,LPCTSTR lpExceptionModuleName,PVOID offsetAddr);
	static void RunRexport(LPCTSTR lpPara);

private:
	static TCHAR m_processVersion[20];
	static TCHAR m_processName[60];
	static TCHAR m_reportExePath[MAX_PATH];
	static BOOL m_bStartHardAcc;
};

#endif