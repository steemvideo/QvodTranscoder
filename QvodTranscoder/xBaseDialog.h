#ifndef XBASEDIALOG_H
#define XBASEDIALOG_H

#include <QDialog>
#include "xTitleBar_ui.h"

class xBaseDialog : public QDialog
{
	Q_OBJECT

public:
	xBaseDialog(QWidget *parent);
	~xBaseDialog();

	void setTitle(QString t);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);
	void resizeEvent ( QResizeEvent *);

protected slots:
	virtual void on_close_clicked();

private:
	void setupUi();

protected:
	QPoint			mDragPosition;
	bool			mbMoveable;
	xTitleBar_ui	*mTitleBar;
};

#endif // XBASEDIALOG_H
