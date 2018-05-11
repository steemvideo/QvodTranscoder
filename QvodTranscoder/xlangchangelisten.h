#ifndef XLANGCHANGELISTEN_H
#define XLANGCHANGELISTEN_H

#include <QObject>

class xLangChangeListen
{
public:
	xLangChangeListen();
	~xLangChangeListen();

	virtual void retranslateUi() {};//语言改变后重新设置各个控件的文本
};

#endif // XLANGCHANGELISTEN_H
