#ifndef XMEDIAINFO_UI_H
#define XMEDIAINFO_UI_H

#include "xBaseDialog.h"
#include "ui_xMediaInfo.h"
#include "xlangchangelisten.h"
struct TRANSCODE_CONFIG;
class xMediaInfo_ui : public xBaseDialog, public xLangChangeListen
{
	Q_OBJECT

public:
	xMediaInfo_ui(QWidget *parent = 0);
	~xMediaInfo_ui();
	virtual void retranslateUi();
	void	set_media_info(boost::shared_ptr<Qvod_DetailMediaInfo> pMediaInfo,const TRANSCODE_CONFIG& config);
private:
	Ui::xMediaInfoClass ui;

private slots:
	void on_btnOK_clicked();
};

#endif // XMEDIAINFO_UI_H
