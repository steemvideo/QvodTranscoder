#ifndef XUPLOAD_UI_H
#define XUPLOAD_UI_H

#include <boost/thread.hpp>
#include "xBaseDialog.h"
#include "ui_xUpload_ui.h"
#include <QStandardItemModel>

#include "xlangchangelisten.h"

class xUploadItemDelegate;
class xUpload_ui : public xBaseDialog, public xLangChangeListen
{
	Q_OBJECT

public:
	xUpload_ui(QWidget *parent = 0);
	~xUpload_ui();
	virtual void retranslateUi();
	void set_output_directory(QString path) { mOutputDir = path;}
signals:
	void sig_AsynFreshRow(int row);
	void sig_AsynComplete();
	void sig_requestUserCMD();

private:
	Ui::xUpload_uiClass ui;
	QStandardItemModel	*mpModel;
	xUploadItemDelegate*	mpItemDelegate;

	QString					mOutputDir;
	QString					mDestPath;
	boost::shared_ptr<boost::thread> mThread;
	bool					mbCanceled;
	bool					mbQuitThread;
	bool					mbReciveUserCMD;//請求用戶決定
	bool					mbCover;//如果存在此文件則覆蓋
protected:
	void add_row(QString url);
	bool eventFilter(QObject *obj, QEvent *event);
	
	void process_copy();
	void int_logical_driver();
private slots:
	void on_btnCopy_clicked();
	void on_btnOpenDirectory_clicked();
	void on_btnSelectDirectory_clicked();
	void on_btnQuit_clicked();
	void slot_AsynFreshRow(int row);
	void slot_AsynComplete();
	void slot_DeviceChanged(bool,QString);
	void slot_requestUserCMD();

};

#endif // XUPLOAD_UI_H
