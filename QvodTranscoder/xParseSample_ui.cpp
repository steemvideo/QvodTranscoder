#include "xParseSample_ui.h"
#include "xParameterEx_ui.h"
#include "commonfunction.h"
#include "xMessageBox_ui.h"
#include "xConfigManager.h"
#include "xFunction.h"

#include <QFileDialog>

xParseSample_ui::xParseSample_ui(QWidget *parent)
	: xBaseDialog(parent)
{
	ui.setupUi(this);
	setTitle(tr("Analyse"));
	mTitleBar->disable_mini_button();

	QStringList models = xConfigManager::Instance().GetModelByDevicesID(xConfigManager::Instance().GetCustomizedDeviceID());
	TranslateQList(models);
	ui.comboBox->addItems(models);
}

xParseSample_ui::~xParseSample_ui()
{
	
}

void xParseSample_ui:: retranslateUi()
{
	ui.retranslateUi(this);
	setTitle(tr("Analyse"));
	int n = ui.comboBox->count();
	for(int i = 0; i < n; i++)
	{
		ui.comboBox->removeItem(0);
	}
	QStringList models = xConfigManager::Instance().GetModelByDevicesID(xConfigManager::Instance().GetCustomizedDeviceID());
	TranslateQList(models);
	ui.comboBox->addItems(models);
}

void xParseSample_ui::on_btnCancel_clicked()
{
	reject();
}

void xParseSample_ui::on_btnParse_clicked()
{

	if (ui.lineEdit->text().isEmpty())
	{
		on_btnSelectDirectory_clicked();
		
		if (ui.lineEdit->text().isEmpty())
		{
			return;
		}
	}

	QString fileName = ui.lineEdit->text();
 	mpMediaInfo.reset(new Qvod_DetailMediaInfo);
 	GetMediaInfo(fileName.toStdWString().c_str(), mpMediaInfo);
 	E_CONTAINER	container = ContainerFromString(mpMediaInfo->container);
 	if (container == _E_CON_OTHER || container == _E_CON_UNKNOW)
 	{
  		xMessageBox box(this);
  		box.SetTitle(tr("Tips"));
  		box.SetText(tr("Sample analysis failed! Please try another samples. You can also use a custom format."));
  		box.SetButtonVisabled(true,false);
  		box.exec();
  		return;
 	}

	//格式化
	QString format = g_toFormatString(QString::fromWCharArray(mpMediaInfo->container));
	if (format == QString("Unknown"))
	{
		format = tr("Unknown");
	}
	
	QString vFormat;
	if(QString::fromWCharArray(mpMediaInfo->videoInfo[0]->Codec).isEmpty())
	{
		vFormat = tr("Invalid");
	}
	else
	{
		vFormat = g_toVideoFormatString(QString::fromWCharArray(mpMediaInfo->videoInfo[0]->Codec));
		if (vFormat == QString("Unknown"))
			vFormat = tr("Unknown");
	}
	
	QString aFormat;
	if(QString::fromWCharArray(mpMediaInfo->audioInfo[0]->Codec).isEmpty())
	{
		aFormat = tr("Invalid");
	}
	else
	{
		aFormat = g_toAudioFormatString(QString::fromWCharArray(mpMediaInfo->audioInfo[0]->Codec));
		if (aFormat == QString("Unknown"))
			aFormat = tr("Unknown");
	}
	
	QString res = QString::number(mpMediaInfo->videoInfo[0]->nWidth) + QString("x") + QString::number(mpMediaInfo->videoInfo[0]->nHeight);
	if (res == QString("0x0"))
		res = tr("Invalid");

	QString fps;
	if (!mpMediaInfo->videoInfo[0]->nFrameRate && !QString::fromWCharArray(mpMediaInfo->videoInfo[0]->Codec).isEmpty())
	{
		mpMediaInfo->videoInfo[0]->nFrameRate = 25;//default
	}
	if (!mpMediaInfo->videoInfo[0]->nFrameRate)
	{
		fps = tr("Invalid");
	}
	else
	{
		fps = QString::number(mpMediaInfo->videoInfo[0]->nFrameRate);
	}

	QString channel;
	if (!mpMediaInfo->audioInfo[0]->nChannel)
	{
		channel = tr("Invalid");
	}
	else
	{
		channel = QString::number(mpMediaInfo->audioInfo[0]->nChannel);
	}

	QString vb;
	if (!mpMediaInfo->videoInfo[0]->nBitrate)
	{
		vb = tr("Invalid");
	}
	else
	{
		vb = QString::number(mpMediaInfo->videoInfo[0]->nBitrate / (float)1000, 'f',1) + QString("Kbps");
	}

	QString ab;
	if (!mpMediaInfo->audioInfo[0]->nBitrate)
	{
		ab = tr("Invalid");
	}
	else
	{
		ab = QString::number(mpMediaInfo->audioInfo[0]->nBitrate / (float)1000, 'f',1)  + QString("Kbps");
	}

	ui.lblType->setText(format);
	ui.lblVideoType->setText(vFormat);
	ui.lblAudioType->setText(aFormat);
	ui.lblRate->setText(fps);
	ui.lblChannel_->setText(channel);
	ui.lblResolution->setText(res);
	ui.lblVBitrate->setText(vb);
	ui.lblABitrate->setText(ab);
}

void xParseSample_ui::on_btnSelectDirectory_clicked()
{
	mFileDialog = new QFileDialog(this);
	QString fileName = mFileDialog->getOpenFileName( this,tr("Select one or more files to open"),"",tr("Media files (*.*)"));
	if (!fileName.isEmpty())
		ui.lineEdit->setText(QDir::toNativeSeparators(fileName));

	delete mFileDialog;
	mFileDialog = NULL;
}

void xParseSample_ui::on_btnOK_clicked()
{
	if (!mpMediaInfo)
	{
		if (!ui.lineEdit->text().isEmpty())
		{
			on_btnParse_clicked();
		}
	}

	if (mpMediaInfo)
	{
		QString format = g_toFormatString(QString::fromWCharArray(mpMediaInfo->container));
		if (format == QString("WM"))
		{
			if (QString::fromWCharArray(mpMediaInfo->videoInfo[0]->Codec).isEmpty())
			{
				format = QString("WMA");
			}
			else
			{
				format = QString("WMV");
			}
		}

		QString vFormat = g_toVideoFormatString(QString::fromWCharArray(mpMediaInfo->videoInfo[0]->Codec));
		QString aFormat = g_toAudioFormatString(QString::fromWCharArray(mpMediaInfo->audioInfo[0]->Codec));
		if ( (format != QString("MP4")
				&& format != QString("MOV")
				&& format != QString("3GP")
				&& format != QString("AVI")
				&& format != QString("WMV")
				&& format != QString("MP3")
				&& format != QString("WMA")
				)
			|| (vFormat != QString("MPEG4") 
				&& vFormat != QString("H264")
				&& vFormat != QString("H263")
				&& vFormat != QString("WMV")
				&& vFormat != QString("MPEG2")
				&& vFormat != QString("Unknown")
				)
			|| (aFormat != QString("AAC")
				&& aFormat != QString("MP3")
				&& aFormat != QString("AC3")
				&& aFormat != QString("AMR_NB")
				&& aFormat != QString("WMA")
				&& aFormat != QString("Unknown")
				)
			)
		{
			xMessageBox box(this);
			box.SetTitle(tr("Tips"));
			box.SetText(tr("unsoupported format!"));
			box.SetButtonVisabled(true,false);
			box.exec();
			return;
		}

		//增加或修改 我的设备
		DEVICE_CONFIGURE mConfig;
		mConfig.model_id = ui.comboBox->currentIndex();
		mConfig.fileFormats << format;
		if (vFormat == QString("Unknown"))
			vFormat = "Invalid";
		if (aFormat == QString("Unknown"))
			aFormat = "Invalid";

		mConfig.vFormats << vFormat;
		mConfig.aFormats << aFormat; 

		QString res = QString::number(mpMediaInfo->videoInfo[0]->nWidth) + QString("x") + QString::number(mpMediaInfo->videoInfo[0]->nHeight);
		if (res == QString("0x0"))
			res = "Invalid";
		mConfig.vResolutions << res;

		mConfig.vFPS  = mpMediaInfo->videoInfo[0]->nFrameRate;
		mConfig.vBitrate = mpMediaInfo->videoInfo[0]->nBitrate;
		mConfig.aBitrate = mpMediaInfo->audioInfo[0]->nBitrate;
		mConfig.aSamplings = mpMediaInfo->audioInfo[0]->nSamplingRate;
		mConfig.aChannels = mpMediaInfo->audioInfo[0]->nChannel;
		if (mConfig.aChannels > 2)
			mConfig.aChannels = 2;

		xConfigManager::Instance().ModifyCustumDevice(mConfig);

		accept();
		return;
	}
	reject();
}

int xParseSample_ui::getCustomizedModelID()
{
	return ui.comboBox->currentIndex();
}
