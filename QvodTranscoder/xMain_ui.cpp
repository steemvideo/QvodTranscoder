#include "xMain_ui.h"


void xMain_UI::setupUi(QWidget *pxTranscoder)
{
	if (pxTranscoder->objectName().isEmpty())
		pxTranscoder->setObjectName(QString::fromUtf8("pxTranscoder"));

	wnd_title = new xTitleBar_ui(pxTranscoder);
	wnd_title->setObjectName(QString::fromUtf8("wnd_title"));
	wnd_title->set_title(QObject::tr("QvodTranscoder"));


	wnd_bottom = new xBottom_ui(pxTranscoder);



	wnd_table = new xTableView_ui(pxTranscoder);
	wnd_table->setObjectName(QString::fromUtf8("wnd_table"));
	wnd_table->setMinimumSize(QSize(600,297));
 	wnd_table->setMaximumSize(QSize(600,297));
	

	// 输出设置
	wnd_parameter = new xParameter_ui(pxTranscoder);

	// 详细输出设置
	wnd_parameter_detail = new xParameterEx_ui(pxTranscoder);
	

	wnd_parameter_detail->hide();

	// 装载设备及型号
	wnd_parameter->load_param();

	wnd_tools = new xTools_ui(pxTranscoder);

 	wnd_list = new xListView_ui(pxTranscoder);
 	wnd_list->setObjectName(QString::fromUtf8("wnd_list"));
	wnd_list->setMinimumSize(QSize(600,297));
	wnd_list->setMaximumSize(QSize(600,297));

	frame_line = new QFrame(pxTranscoder);
	frame_line->setObjectName(QString::fromUtf8("line"));
	frame_line->setFrameShape(QFrame::HLine);
	frame_line->setFrameShadow(QFrame::Sunken);

	// 输出设置
	wnd_output = new xOutput_ui(pxTranscoder);

	QHBoxLayout* layout_of_stacked_layout = new QHBoxLayout();
	layout_of_stacked_layout->setContentsMargins(0, 0, 0, 0);

	stacked_layout = new QStackedLayout;
	stacked_layout->addWidget(wnd_table);
	stacked_layout->addWidget(wnd_list);

	layout_of_stacked_layout->addLayout(stacked_layout);

	main_layout = new QVBoxLayout(pxTranscoder);
	main_layout->setContentsMargins(10, 1, 10, 1);
	main_layout->setSpacing(2);


	main_layout->addWidget(wnd_title);
	main_layout->addWidget(wnd_tools);
	main_layout->addLayout(layout_of_stacked_layout);
	main_layout->addWidget(wnd_parameter);
	main_layout->addWidget(wnd_parameter_detail);
	main_layout->addWidget(wnd_output);
	main_layout->addWidget(frame_line);
	main_layout->addWidget(wnd_bottom);

	pxTranscoder->setLayout(main_layout);
	pxTranscoder->resize(620, 445);

	mpTranscoder = pxTranscoder;
} // setupUi

void xMain_UI::retranslateUi()
{
	wnd_title->set_title(QObject::tr("QvodTranscoder"));
	mpTranscoder->setWindowTitle(QCoreApplication::translate("QvodTranscoderClass","QvodTranscoder"));
}