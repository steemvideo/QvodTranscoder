#include "xTester_ui.h"
#include "xTaskManagerUI.h"

xTester_ui::xTester_ui(xTaskManagerUI* pTM,QWidget *parent)
	: xBaseDialog(parent)
	, mpTaskManager(pTM)
{
	ui.setupUi(this);
	mTitleBar->disable_mini_button();
	setTitle(tr("Testing in progress"));

	connect(mpTaskManager,SIGNAL(sig_AsynFresh(int)), this, SLOT(slot_AsynFresh(int)), Qt::QueuedConnection);
	connect(mpTaskManager,SIGNAL(sig_addTask(int)), this, SLOT(slot_addTask(int)));
	connect(mpTaskManager,SIGNAL(sig_engine_states_changed(E_ENGINE_STATES)), this, SLOT(slot_engine_states_changed(E_ENGINE_STATES)));
}

xTester_ui::~xTester_ui()
{
}

void xTester_ui::retranslateUi()
{
	ui.retranslateUi(this);
	setTitle(tr("Testing in progress"));
}

void xTester_ui::paintEvent(QPaintEvent* event)
{
	xBaseDialog::paintEvent(event);

	QPainter painter(this);
	// ½ø¶È
	xTaskItemPtr item = mpTaskManager->get_item_by_index(0);
	if (item)
	{
		painter.setRenderHints(QPainter::HighQualityAntialiasing);

		// draw background
		QLinearGradient gradient(QPoint(0,0),QPoint(0,10));
		gradient.setColorAt(0,QColor(59,59,59));
		gradient.setColorAt(0.5,QColor(99,99,99));
		gradient.setColorAt(1,QColor(59,59,59));
		QBrush brush(gradient);
		painter.setBrush(brush);

		QRect rect_bg(10,55,width() - 20, 20);
		painter.drawRoundedRect(rect_bg, 3,3);

		// draw progress
		rect_bg.adjust(1,1,-1,-1);
		float pp = item->get_progress() / 10000.0f;
		rect_bg.setWidth(pp * rect_bg.width());

		QLinearGradient gradient2(QPoint(0,0),QPoint(0,10));
		gradient2.setColorAt(0,QColor(242,245,236));
		gradient2.setColorAt(0.5,QColor(145,197,134));
		gradient2.setColorAt(1,QColor(152,196,137));
		QBrush brush2(gradient2);
		painter.setBrush(brush2);
		painter.drawRect(rect_bg);
	}
}

void xTester_ui::slot_AsynFresh(int)
{
	update();
}

void xTester_ui::slot_addTask(int task_id)
{
	xTaskItemPtr item = mpTaskManager->get_item_by_task_id(task_id);
	if (item)
	{
		QFont font(ui.label_2->font());
		QFontMetrics fm(font);
		ui.label_2->setText(fm.elidedText(item->get_display_text(), Qt::ElideRight, ui.label_2->width()));
	}
}

void xTester_ui::on_bottonClose_clicked()
{
	mpTaskManager->stop();
	reject();
}

void xTester_ui::on_close_clicked()
{
	on_bottonClose_clicked();
}

void xTester_ui::slot_engine_states_changed(E_ENGINE_STATES s)
{
	if(s == ENGINE_STATES_STOP)
	{
		accept();
	}
}