#ifndef XTOOLS_UI_H
#define XTOOLS_UI_H

#include <QWidget>
#include "ui_xTools.h"
#include "xlangchangelisten.h"
class xTools_ui : public QWidget, public xLangChangeListen
{
	Q_OBJECT

public:
	xTools_ui(QWidget *parent = 0);
	~xTools_ui();

	virtual void retranslateUi();
signals:
	void sig_import();
	void sig_up();
	void sig_down();
	void sig_remove();
	void sig_modelChanged(bool);

public slots:
	void slot_select_changed(int);

private:
	Ui::xToolsClass ui;
	bool			mbListMode;
private slots:
	void on_btnImport_clicked();
	void on_btnViewMode_clicked();
	void on_btnDown_clicked();
	void on_btnUp_clicked();
	void on_btnRemove_clicked();
};

#endif // XTOOLS_UI_H
