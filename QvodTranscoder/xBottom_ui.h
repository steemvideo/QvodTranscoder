#ifndef XBOTTOM_UI_H
#define XBOTTOM_UI_H

#include <QWidget>
#include "ui_xBottom.h"
#include "xlangchangelisten.h"
class xBottom_ui : public QWidget, public xLangChangeListen
{
	Q_OBJECT

public:
	xBottom_ui(QWidget *parent = 0);
	~xBottom_ui();
	virtual void retranslateUi();
	bool is_shutdown_checked() const;
signals:
	void sig_stop();
	void sig_start();
	void sig_pause();

public slots:
	void slot_engine_statusChanged(E_ENGINE_STATES);
	void slot_item_changed(int);

private:
	Ui::xBottom_uiClass ui;
private slots:
	void on_btnBegin_clicked();
	void on_btnStop_clicked();
};

#endif // XBOTTOM_UI_H
