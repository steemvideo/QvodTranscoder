#pragma once

#include "xlangchangelisten.h"
#include "xFunction.h"
#include "xTranscoder.h"

class QvodApplication : public QApplication
{
	Q_OBJECT
public:
	static QString g_CurrentDirectory;
	static QvodApplication* instance()
	{
		return self;
	}
	xTranscoder* mMainWnd;

	QvodApplication( int argc, char ** argv, int lang = 0/*”Ô—‘*/ );
	void addLangChangeListen(xLangChangeListen* listen);
	void removeLangChangeListen(xLangChangeListen* listen);
	void set_current_lang(int lang);

signals:
	void sig_DeviceChanged(bool,QString);
protected:
	void get_current_directory();
	bool winEventFilter ( MSG * msg, long * result );

protected:
	QTranslator* mTS;	
	QTranslator* mTSQT;
	static QvodApplication* self;
	std::list<xLangChangeListen*>	mListenner;

};
