#include "stdafx.h"
#include <QtGui/QApplication>
#include "xTranscoder.h"
#include <QTranslator>
#include "QtWin.h"
#include "xMessageBox_ui.h"
#include "xFunction.h"
#include "xApp.h"
#include "Coredump.h"

extern "C" __declspec(dllimport) bool InitQvodTaskManager(void** ppTaskManager);
extern "C" __declspec(dllimport) void DestroyQvodTaskManager(void* ppTaskManager);

// 初始化全局
ITaskManager*	g_Taskmanager = NULL;
QString QvodApplication::g_CurrentDirectory = QString();
QvodApplication* QvodApplication::self = NULL;

bool GetQvodCrashPath(TCHAR* path)  
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
	RegCloseKey(hKEY);   
	return true;  
}  


int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	// 激活已有进程?
	QSharedMemory mem_hwnd("QvodTranscoder_HWND");

	if (!mem_hwnd.create(sizeof(HWND)))
	{
		mem_hwnd.attach();
		mem_hwnd.lock();
		HWND w = HWND(*((HWND*)mem_hwnd.data()));
		mem_hwnd.unlock();

		QSharedMemory mem_cmd("QvodTranscoder_cmd");
		if (!mem_cmd.create(MAX_SHARED_MEMORY))
		{
			SetForegroundWindow(w);

			SetWindowPos(w,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
			ShowWindow(w, SW_SHOWNORMAL);

			SetWindowPos(w,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

			// URL追加
			mem_cmd.attach();
			mem_cmd.lock();
			memset(mem_cmd.data(),0,MAX_SHARED_MEMORY);
			int count = min(_tcslen(lpCmdLine)*2,MAX_SHARED_MEMORY);
			memcpy(mem_cmd.data(),lpCmdLine,count);
			mem_cmd.unlock();

			SendMessage(w,WM_CMDLINE_ACTIVE,0,0);
			
		}
		return 0;
	}

	TCHAR current_path[512];
	if(GetQvodCrashPath(current_path))
	{
		CString processName = "QvodTranscoder";
		CString processVersion = "1.2.0.1";
		CString reportExePath = current_path;
		reportExePath += "\\CrashReport.exe";
		CoreDump::SetExceptionHandle( processName,processVersion,reportExePath);
	}
	

	// 创建进程间传递命令行用的缓冲区
	QSharedMemory mem_cmd("QvodTranscoder_cmd");
	mem_cmd.create(MAX_SHARED_MEMORY);

	
		
	// 命令行
	QStringList urls;
	QString cmdline = QString::fromWCharArray(lpCmdLine);
 #ifndef QVOD_TRANSCODER_ONLY	
 	if (!cmdline.contains(QString("-qvod")))
 	{	
 		// 限制只能由播放器启动
 		return 0;
 	}
 #endif
	int lan = 0;
	g_parse_cmdline(cmdline, urls, &lan);
	int argc = 1;
	QvodApplication _app(argc, NULL, lan);

	//获取模块目录
	if(!QDir::setCurrent(QvodApplication::g_CurrentDirectory))
	{
		qDebug() << "SET REF DIR FAILED!!!";
	}
	
	// 注册类型
	qRegisterMetaType<E_ENGINE_STATES>("E_ENGINE_STATES");

	// 加载皮肤+
	QFile file(QvodApplication::g_CurrentDirectory + "main.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	qApp->setStyleSheet(styleSheet);

	// 主线程循环
	HANDLE hMutex = CreateMutex(NULL, FALSE, L"QVodTranscoder");//用于播放器判断转码器是否开启
	#pragma region
	{
		InitQvodTaskManager((void**)&g_Taskmanager);
		{
			xTranscoder window(g_Taskmanager);
			//显示
			window.show(); 
			_app.mMainWnd = &window;
			if (urls.size())
			{
				window.load_urls(urls);
			}

			HWND w = window.winId();
			//设置进程优先级
			{
				DWORD dwProcess = 0;
				GetWindowThreadProcessId(w, &dwProcess);
				if(dwProcess)
				{
					HANDLE hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, dwProcess);
					if(hProcess)
					{
						SetPriorityClass(hProcess, BELOW_NORMAL_PRIORITY_CLASS);
					}
					CloseHandle(hProcess);
				}		
			}

			//主窗口句柄进程间共享
			{	
				mem_hwnd.attach();
				mem_hwnd.lock();
				memcpy(mem_hwnd.data(),&w,sizeof(HWND));
				mem_hwnd.unlock();
			}

			_app.exec();

		}
		DestroyQvodTaskManager((void*)g_Taskmanager);
		g_Taskmanager = NULL;
	}

	CloseHandle(hMutex);
	#pragma endregion
	return 0;
}

