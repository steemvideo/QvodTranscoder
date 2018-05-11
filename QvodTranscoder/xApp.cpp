#include "xApp.h"
#include <dbt.h>

QvodApplication::QvodApplication( int argc, char ** argv, int lang) 
:QApplication( argc, argv )
, mTSQT(NULL)
, mTS(NULL)
, mMainWnd(NULL)
{
	self = this;

	get_current_directory();

	// 设置语言
	set_current_lang(lang);
}

// 获取exe所在路径字符串
void QvodApplication::get_current_directory()
{
	g_CurrentDirectory = QCoreApplication::applicationDirPath();
	g_CurrentDirectory = QDir::toNativeSeparators(g_CurrentDirectory);
	if (g_CurrentDirectory.right(1) != QString("\\"))
	{
		g_CurrentDirectory += QString("\\");
	}
}

void QvodApplication::set_current_lang(int lang)
{
	if (mTS)
	{
		removeTranslator(mTS);
		delete mTS;
		mTS = NULL;
	}

	if (mTSQT)
	{
		removeTranslator(mTSQT);
		delete mTSQT;
		mTSQT = NULL;
	}

	// 简体中文
	if(lang == 0)
	{
		QString tsUrl = g_CurrentDirectory + "TS\\qvodtranscoder_zh.qm";
		mTS = new QTranslator;
		bool bOK = mTS->load(tsUrl);
		installTranslator(mTS);

		tsUrl = g_CurrentDirectory + "TS\\qt_zh_CN.qm";
		mTSQT = new QTranslator;
		bOK = mTSQT->load(tsUrl);
		installTranslator(mTSQT);
	}
	// 繁体中文
	else if(lang == 1)
	{
		QString tsUrl = g_CurrentDirectory + "TS\\qvodtranscoder_tw.qm";
		mTS = new QTranslator;
		mTS->load(tsUrl);
		installTranslator(mTS);

		tsUrl = g_CurrentDirectory + "TS\\qt_zh_TW.qm";
		mTSQT = new QTranslator;
		mTSQT->load(tsUrl);
		installTranslator(mTSQT);
	}
	fontDatabaseChanged();


	// 通知每个活动子窗口更新自己的UI文本
	for(std::list<xLangChangeListen*>::iterator pos = mListenner.begin(); pos != mListenner.end(); pos++)
		(*pos)->retranslateUi();
}

void QvodApplication::addLangChangeListen(xLangChangeListen* listen)
{
	mListenner.push_back(listen);
}

void QvodApplication::removeLangChangeListen(xLangChangeListen* listen)
{
	mListenner.remove(listen);
}

bool QvodApplication::winEventFilter ( MSG * msg, long * result )
{
	if (msg->message == WM_CMDLINE_ACTIVE)
	{
		QSharedMemory mem("QvodTranscoder_cmd");
		if (!mem.create(MAX_SHARED_MEMORY))
		{
			mem.attach();
			mem.lock();
			wchar_t* d = (wchar_t*)mem.data();
			QString cmdline = QString::fromWCharArray(d);
			QStringList urls;
			int language = -1;
			g_parse_cmdline(cmdline,urls, &language);
			if (urls.size())
			{
				mMainWnd->load_urls(urls);
			}
			mem.unlock();
			mem.detach();

			//切换语言
			if (language != -1)
			{
				set_current_lang(language);
			}
		}
		*result = true;
		return true;
	}
	else if(msg->message == WM_DEVICECHANGE)
	{
		//移动设备
		switch (msg->wParam)
		{ 
		case DBT_DEVICEARRIVAL:	
		case DBT_DEVICEREMOVECOMPLETE:
			{
				PDEV_BROADCAST_HDR pstDevHdr = (PDEV_BROADCAST_HDR)msg->lParam;
				if(pstDevHdr->dbch_devicetype == DBT_DEVTYP_VOLUME)
				{
					PDEV_BROADCAST_VOLUME pstDev = (PDEV_BROADCAST_VOLUME)msg->lParam;
					if(pstDev->dbcv_flags == 0)
					{
						for (int i = 0; i < 32; i++)
						{
							if(((1 << i) & pstDev->dbcv_unitmask))
							{
								QString str = QChar('A' + i);
								str += ":\\";

								emit sig_DeviceChanged(msg->wParam == DBT_DEVICEARRIVAL, str);
							}
						}
					}
				}
			}
			break;
		}
	}

	return false;
}