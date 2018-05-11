#include "xlangchangelisten.h"
#include "xApp.h"

xLangChangeListen::xLangChangeListen()
{
	QvodApplication::instance()->addLangChangeListen(this);
}

xLangChangeListen::~xLangChangeListen()
{
	QvodApplication::instance()->removeLangChangeListen(this);
}
