#include "xBottom_ui.h"
#include "xApp.h"

xBottom_ui::xBottom_ui(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.btnStop->setEnabled(false);
	ui.cbxShutDown->hide();
}

xBottom_ui::~xBottom_ui()
{

}

void xBottom_ui::retranslateUi()
{
	ui.retranslateUi(this);
	if ( QvodApplication::instance()->mMainWnd)
	{
		E_ENGINE_STATES s = QvodApplication::instance()->mMainWnd->get_taskManager()->get_engine_states();
		slot_engine_statusChanged(s);
	}
	
}

void xBottom_ui::on_btnStop_clicked()
{
	emit sig_stop();
}

void xBottom_ui::on_btnBegin_clicked()
{
	emit sig_start();
}

void xBottom_ui::slot_item_changed(int c)
{
	if (!c)
	{
		ui.btnBegin->setText(tr("Begin"));
		ui.btnBegin->setEnabled(true);
		ui.btnStop->setEnabled(false);
	}
}

void xBottom_ui::slot_engine_statusChanged(E_ENGINE_STATES s )
{
	if (s == ENGINE_STATES_TRANSCODING)
	{
		ui.btnBegin->setText(tr("Pause"));
		ui.btnStop->setEnabled(true);
	}
	else if(s == ENGINE_STATES_PAUSE)
	{
		ui.btnBegin->setText(tr("Continue"));
		ui.btnBegin->setEnabled(true);
		ui.btnStop->setEnabled(true);
	}
	else if(s == ENGINE_STATES_STOP)
	{
		ui.btnBegin->setText(tr("Begin"));
		ui.btnBegin->setEnabled(true);
		ui.btnStop->setEnabled(false);
	}
}

bool xBottom_ui::is_shutdown_checked() const
{
	return ui.cbxShutDown->isChecked();
}