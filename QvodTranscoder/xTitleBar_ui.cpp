#include "xTitleBar_ui.h"

xTitleBar_ui::xTitleBar_ui(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

xTitleBar_ui::~xTitleBar_ui()
{

}


void xTitleBar_ui::on_btnMin_clicked()
{
	emit sig_min();
}

void xTitleBar_ui::on_btnClose_clicked()
{
	emit sig_close();
}

void xTitleBar_ui::set_title(QString txt)
{
	ui.lblText->setText(txt);
}

void xTitleBar_ui::set_pixmap(QPixmap p)
{
	ui.lblText->setPixmap(p);
}

void xTitleBar_ui::showEvent(QShowEvent *)
{
	if (!ui.btnMin->isHidden())
	{
		ui.btnMin->hide();
		ui.btnMin->show();
	}
}

void xTitleBar_ui::disable_mini_button()
{
	//ui.btnMin->setEnabled(false);
	ui.btnMin->hide();
	ui.btnClose->setObjectName("btnClose1");
	ui.btnClose->update();
}