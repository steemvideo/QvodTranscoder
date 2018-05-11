#ifndef XOUTPUT_UI_H
#define XOUTPUT_UI_H

#include <QWidget>
#include "ui_xOutput_ui.h"
#include "xlangchangelisten.h"
class xOutput_ui : public QWidget, public xLangChangeListen
{
	Q_OBJECT

public:
	xOutput_ui(QWidget *parent = 0);
	~xOutput_ui();

	QString getOutputDir();
	void	save_to_setting(QSettings& set);
	void	load_from_setting(QSettings& set);

	virtual void retranslateUi();
private:
	Ui::xOutputClass ui;

private slots:
	void on_btnUpload_clicked();
	void on_btnOpenDirectory_clicked();
	void on_btnSelectDirectory_clicked();
	void slot_engine_statusChanged(E_ENGINE_STATES s);
};

#endif // XOUTPUT_UI_H
