#ifndef XMEDIAINFO_UI_H
#define XMEDIAINFO_UI_H

#include "xBaseDialog.h"
#include "ui_xParseSample.h"
#include "xlangchangelisten.h"

class Qvod_DetailMediaInfo;
struct DEVICE_CONFIGURE;
class xParseSample_ui : public xBaseDialog, public xLangChangeListen
{
	Q_OBJECT

public:
	xParseSample_ui(QWidget *parent = 0);
	~xParseSample_ui();

	int		getCustomizedModelID();
	virtual void retranslateUi();
private:
	Ui::xParseSampleClass ui;
	QFileDialog*	mFileDialog;
private slots:
	void on_btnSelectDirectory_clicked();
	void on_btnParse_clicked();
	void on_btnCancel_clicked();
	void on_btnOK_clicked();
protected:
	boost::shared_ptr<Qvod_DetailMediaInfo> mpMediaInfo;
};

#endif // XMEDIAINFO_UI_H
