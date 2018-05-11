#pragma once
#include "xTableView_ui.h"
#include "xParameter_ui.h"
#include "xParameterEx_ui.h"
#include "xTitleBar_ui.h"
#include "xOutput_ui.h"
#include "xBottom_ui.h"
#include "xTools_ui.h"
#include "xListView_ui.h"

class xMain_UI:public xLangChangeListen
{
public:
	xTitleBar_ui*wnd_title;
	xBottom_ui*	 wnd_bottom;

	xTableView_ui	*wnd_table;
	xListView_ui	*wnd_list;
	xParameter_ui *wnd_parameter;
	xParameterEx_ui* wnd_parameter_detail;

	xTools_ui	*wnd_tools;

	QFrame		*frame_line;
	xOutput_ui	*wnd_output;
		
	QStackedLayout*	stacked_layout;
	QVBoxLayout*	main_layout;
	QWidget*	mpTranscoder;
public:
	void setupUi(QWidget *pxTranscoder);
	void retranslateUi();
};