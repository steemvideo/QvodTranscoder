#include "xTools_ui.h"

xTools_ui::xTools_ui(QWidget *parent)
	: QWidget(parent)
	, mbListMode(true)
{
	ui.setupUi(this);

	ui.btnDown->setEnabled(false);
	ui.btnDown->setToolTip(tr("Down"));
	ui.btnUp->setEnabled(false);
	ui.btnUp->setToolTip(tr("Up"));
	ui.btnRemove->setEnabled(false);
	ui.btnRemove->setToolTip(tr("Remove"));

	ui.btnViewMode->setToolTip(tr("Switch display mode"));
}

xTools_ui::~xTools_ui()
{

}

void xTools_ui::retranslateUi()
{
	ui.retranslateUi(this);
	ui.btnDown->setToolTip(tr("Down"));
	ui.btnUp->setToolTip(tr("Up"));
	ui.btnRemove->setToolTip(tr("Remove"));
	ui.btnViewMode->setToolTip(tr("Switch display mode"));
}

void xTools_ui::slot_select_changed(int cout)
{
	ui.btnDown->setEnabled(cout == 1);
	ui.btnUp->setEnabled(cout == 1);
	ui.btnRemove->setEnabled(cout > 0);
}

void xTools_ui::on_btnRemove_clicked()
{
	emit sig_remove();
}

void xTools_ui::on_btnUp_clicked()
{
	emit sig_up();
}

void xTools_ui::on_btnDown_clicked()
{
	emit sig_down();
}

void xTools_ui::on_btnViewMode_clicked()
{
	mbListMode = !mbListMode;
	emit sig_modelChanged(mbListMode);
}

void xTools_ui::on_btnImport_clicked()
{
	emit sig_import();
}