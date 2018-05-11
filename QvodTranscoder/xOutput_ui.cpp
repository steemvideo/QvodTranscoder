#include "xOutput_ui.h"
#include <QFileDialog>
#include <QDir>
#include "xFunction.h"
#include "xUpload_ui.h"
#include "xApp.h"

xOutput_ui::xOutput_ui(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QDir d = QDir::home();
	ui.lineEdit->setText(d.toNativeSeparators(d.absolutePath()));	
	ui.btnSelectDirectory->setToolTip(tr("Select output folder"));
	ui.btnOpenDirectory->setToolTip(tr("Open output folder"));
	ui.btnUpload->setToolTip(tr("Copy the transcoded file to the device"));
	ui.btnUpload->setEnabled(false);
}

xOutput_ui::~xOutput_ui()
{

}

void xOutput_ui::retranslateUi()
{
	ui.retranslateUi(this);
	ui.btnSelectDirectory->setToolTip(tr("Select output folder"));
	ui.btnOpenDirectory->setToolTip(tr("Open output folder"));
	ui.btnUpload->setToolTip(tr("Copy the transcoded file to the device"));
}

void xOutput_ui::on_btnSelectDirectory_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder"),
													ui.lineEdit->text(),
													QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (!dir.isEmpty())
	{
		QString d = QDir::toNativeSeparators(dir);
		ui.lineEdit->setText(d);
	}
}

void xOutput_ui::on_btnOpenDirectory_clicked()
{
	if (ui.lineEdit->text().isEmpty())
	{
		return;
	}

	QDir d(ui.lineEdit->text());
	if(!d.exists())
	{
		bool bOK = d.mkpath(ui.lineEdit->text());
		if(!bOK)
		{
			return;
		}
	}

	g_open_dir(ui.lineEdit->text());
}

void xOutput_ui::on_btnUpload_clicked()
{
	xUpload_ui upload(this);
	upload.set_output_directory(ui.lineEdit->text());
	upload.exec();
}

QString xOutput_ui::getOutputDir()
{
	return ui.lineEdit->text();
}

void xOutput_ui::slot_engine_statusChanged(E_ENGINE_STATES s)
{
	if (s == ENGINE_STATES_TRANSCODING || s == ENGINE_STATES_PAUSE)
	{ 
		ui.lineEdit->setEnabled(false);
		ui.btnSelectDirectory->setEnabled(false);
		ui.btnUpload->setEnabled(false);
	}
	else
	{
		ui.lineEdit->setEnabled(true);
		ui.btnSelectDirectory->setEnabled(true);

		if( 1 <= QvodApplication::instance()->mMainWnd->get_taskManager()->get_count_by_status(ITEM_STATES_COMPLETE))
			ui.btnUpload->setEnabled(true);
	}
} 

void xOutput_ui::save_to_setting(QSettings& set)
{
	set.beginGroup("output");
	set.setValue("dir", ui.lineEdit->text()); 
	set.endGroup();
}

void xOutput_ui::load_from_setting(QSettings& set)
{
	set.beginGroup("output");
	ui.lineEdit->setText(set.value("dir").toString()); 
	set.endGroup();

	if (ui.lineEdit->text().isEmpty())
	{
		QDir d = QDir::home();
		ui.lineEdit->setText(d.toNativeSeparators(d.absolutePath()));	
	}
}
