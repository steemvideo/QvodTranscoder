#ifndef XTITLEBAR_UI_H
#define XTITLEBAR_UI_H

#include <QWidget>
#include "ui_xTitleBar.h"

class xTitleBar_ui : public QWidget
{
	Q_OBJECT

public:
	xTitleBar_ui(QWidget *parent = 0);
	~xTitleBar_ui();

	void set_title(QString);
	void set_pixmap(QPixmap p);

	void disable_mini_button();
signals:
	void sig_close();
	void sig_min();

private:
	Ui::xTitleBarClass ui;
private slots:
	void on_btnClose_clicked();
	void on_btnMin_clicked();
	void showEvent(QShowEvent *);
};

#endif // XTITLEBAR_UI_H
