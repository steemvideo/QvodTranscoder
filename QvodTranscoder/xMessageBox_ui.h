#ifndef XMESSAGEBOX_UI_H
#define XMESSAGEBOX_UI_H

#include <QDialog>
#include "ui_xMessageBox.h"
#include "xlangchangelisten.h"
class xMessageBox : public QDialog, public xLangChangeListen
{
	Q_OBJECT

public:
	xMessageBox(QWidget *parent = 0);
	~xMessageBox();

	virtual void retranslateUi();

	void SetTitle(QString);
	void DisableMiniButton();
	void SetText(QString str, QString format = QString(), int auto_close_second = 0);
	void SetButtonVisabled(bool bOK,bool bCancel);
	void SetButtonText(int index, QString);

private:
	Ui::xMessageBoxClass ui;

private slots:
	void on_btnCancel_clicked();
	void on_btnOK_clicked();
	void paintEvent(QPaintEvent* event);
	void slot_timeout();
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent* event);
protected:
	QPoint			mDragPosition;
	bool			mbMoveable;	
	QString			mAutoText;
	QTimer			mTimer;
	int				mSecondToClose;
	int				mSecondCast;
};

#endif // XMESSAGEBOX_UI_H
