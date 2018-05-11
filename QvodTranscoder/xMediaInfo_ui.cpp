#include "xMediaInfo_ui.h"
#include "xParameterEx_ui.h"
#include "xFunction.h"

xMediaInfo_ui::xMediaInfo_ui(QWidget *parent)
	: xBaseDialog(parent)
{
	ui.setupUi(this);
	mTitleBar->disable_mini_button();
	setTitle(tr("Media Info"));
}

xMediaInfo_ui::~xMediaInfo_ui()
{

}

void xMediaInfo_ui::retranslateUi()
{
	ui.retranslateUi(this);
	setTitle(tr("Media Info"));
}

void xMediaInfo_ui::set_media_info(boost::shared_ptr<Qvod_DetailMediaInfo> pMediaInfo,const TRANSCODE_CONFIG& config)
{
	ui.lblType->setText(g_toFormatString(QString::fromWCharArray(pMediaInfo->container)));
	ui.lblRate->setText(QString::number(pMediaInfo->videoInfo[0]->nFrameRate));
	ui.lblResolution->setText(QString::number(pMediaInfo->videoInfo[0]->nWidth) + QString("x") + QString::number(pMediaInfo->videoInfo[0]->nHeight));
	ui.lblVideoType->setText(g_toVideoFormatString(QString::fromWCharArray(pMediaInfo->videoInfo[0]->Codec)));
	ui.lblAudioType->setText(g_toAudioFormatString(QString::fromWCharArray(pMediaInfo->audioInfo[0]->Codec)));
	ui.lblFileSize->setText(QString::number(pMediaInfo->fileSize / (float)1048576, 'f',1) + QString(" MB"));


	ui.lblType_2->setText(g_toFormatString(config.ContainerType));
	ui.lblRate_2->setText(QString::number(config.VideoCodec.framerate));
	ui.lblResolution_2->setText(QString::number(config.VideoCodec.width) + QString("x") + QString::number(config.VideoCodec.height));
	ui.lblVideoType_2->setText(g_toVideoFormatString(config.VideoCodec.CodecType));
	ui.lblAudioType_2->setText(g_toAudioFormatString(config.AudioCodec.CodecType));

	
	int vbit = config.VideoCodec.bitrate;
	int abit = config.AudioCodec.bitrate;

	int total = (vbit + abit) / 8;
	int size = total * pMediaInfo->nDuration / (float)1000;
	ui.lblFileSize_2->setText(QString::number(size/(float)1048576, 'f',1) + QString(" MB"));
}

void xMediaInfo_ui::on_btnOK_clicked()
{
	accept();
}
