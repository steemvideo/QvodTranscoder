#include "xParameterEx_ui.h"
#include "xConfigManager.h"
#include "commonfunction.h"
#include "xFunction.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
void _g_add_to_combobox(QComboBox* box, QString str)
{
	//str.remove(QString(".0"));
	int idx = box->findText(str);
	if (idx != -1)
	{
		box->setCurrentIndex(idx);
	}
	else
	{
		box->addItem(str);
		box->setCurrentIndex(box->count()-1);
	}
}

void _g_add_to_combobox(QComboBox* box, QStringList strList)
{
	box->addItems(strList);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
xParameterEx_ui::xParameterEx_ui(QWidget *parent)
	: QWidget(parent)
	, mbIsCustomized(false)
{
	ui.setupUi(this);

	init_ui();
	
}

xParameterEx_ui::~xParameterEx_ui()
{

}

void xParameterEx_ui::retranslateUi()
{
	ui.retranslateUi(this);
	//ui.btnTry->setToolTip(tr("Try transcoding for 1 min"));
}

void xParameterEx_ui::init_ui()
{
	ui.cbx_format->setEnabled(false);
	connect(ui.cbx_format,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_formatChanged(const QString&)));
	connect(ui.cbx_format,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_encode,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_video_bitrate,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_fps,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_resolution,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_audio_bitrate,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_channels,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.cbx_sample,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(slot_audioVideo_Changed(const QString&)));
	connect(ui.btnCustum,SIGNAL(stateChanged(int)),this,SLOT(on_checkBox_stateChanged(int)));

	connect(ui.cbx_KeepAspect,SIGNAL(stateChanged(int)),this,SLOT(slot_keepAspect_Changed(int)));

	ui.cbx_KeepAspect->setChecked(true);

	// 自定义可选择的格式
	QStringList formats = xConfigManager::Instance().GetPreDefinedFormats();
	ui.cbx_format->clear();
	ui.cbx_format->addItems(formats);
}

void xParameterEx_ui::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect r = rect();

	QPen pen(Qt::gray);
	pen.setWidth(2);
	painter.setPen(pen);

	QBrush brush(QColor(238,238,238));
	painter.setBrush(brush);

	painter.drawRect(r);
}

void xParameterEx_ui::slot_deviceModel_Changed(int device,int model)
{
	mDevice = device;
	mModel = model;
	
	DEVICE_CONFIGURE config;
	xConfigManager::Instance().GetDefaultParam(mDevice, mModel, config);
	if (config.is_customized)
	{	
		mbIsCustomized = true;
		ui.btnCustum->blockSignals(true);
		ui.btnCustum->setChecked(true);
		ui.btnCustum->blockSignals(false);
		on_checkBox_stateChanged(Qt::Checked);
		
		ui.btnCustum->setEnabled(false);
	}
	else
	{
		mbIsCustomized = false;

		ui.btnCustum->setEnabled(true);
		
		ui.btnCustum->blockSignals(true);
		ui.btnCustum->setChecked(false);
		ui.btnCustum->blockSignals(false);
		on_checkBox_stateChanged(Qt::Unchecked);
	}
}

void xParameterEx_ui::on_checkBox_stateChanged(int s)
{
	if (s == Qt::Checked)
	{
		ui.cbx_format->setEnabled(true);
		ui.cbx_encode->setEnabled(true);
		ui.cbx_resolution->setEnabled(true);
		ui.cbx_fps->setEnabled(true);
		ui.cbx_video_bitrate->setEnabled(true);
		ui.cbx_audio_bitrate->setEnabled(true);
		ui.cbx_channels->setEnabled(true);
		ui.cbx_sample->setEnabled(true);
		ui.cbx_KeepAspect->setEnabled(true);

		DEVICE_CONFIGURE config;
		xConfigManager::Instance().GetDefaultParam(mDevice, mModel, config);
		if (config.is_customized)
		{
			set_force_config(config);
		}
		
		if (ui.cbx_format->count() == 0)
		{
			// 自定义設備
			QStringList formats = xConfigManager::Instance().GetPreDefinedFormats();
			ui.cbx_format->clear();
			ui.cbx_format->addItems(formats);
		}
	}
	else
	{
		set_Default();
		ui.cbx_KeepAspect->setEnabled(false);
		ui.cbx_KeepAspect->setChecked(true);
	}
}

void xParameterEx_ui::set_Default()
{
	DEVICE_CONFIGURE config;
	if(xConfigManager::Instance().GetDefaultParam(mDevice, mModel, config))
	{
		set_force_config(config);
	}	

	ui.cbx_KeepAspect->setEnabled(false);
	ui.cbx_encode->setEnabled(false);
	ui.cbx_resolution->setEnabled(false);
	ui.cbx_fps->setEnabled(false);
	ui.cbx_video_bitrate->setEnabled(false);
	ui.cbx_audio_bitrate->setEnabled(false);
	ui.cbx_channels->setEnabled(false);
	ui.cbx_sample->setEnabled(false);
	ui.cbx_format->setEnabled(false);
}

//強制設置
void xParameterEx_ui::set_force_config(DEVICE_CONFIGURE& config)
{
	// 文件格式
	QString format;
	if(config.fileFormats.size() && !config.fileFormats[0].isEmpty())
	{
		format = config.fileFormats[0];
		_g_add_to_combobox(ui.cbx_format,config.fileFormats[0]);
	}
	else
	{
		ui.cbx_format->setCurrentIndex(0);
	}

	// 编码方式
	QString video_str = config.vFormats.size() ? config.vFormats[0] : QString();
	QString audio_str = config.aFormats.size() ? config.aFormats[0] : QString();
	QString encode_str;
	if (!video_str.isEmpty())
	{
		encode_str = video_str;
		if (!audio_str.isEmpty())
		{
			encode_str += "+";
			encode_str += audio_str;
		}
	}
	else if(!audio_str.isEmpty())
	{
		encode_str = audio_str;
	}
	_g_add_to_combobox(ui.cbx_encode, encode_str);


	// 分辨率
	if(config.vResolutions.size())
	{
		if (!config.vResolutions[0].isEmpty())
		{
			_g_add_to_combobox(ui.cbx_resolution,config.vResolutions[0]);
		}
	}
	else
	{
		_g_add_to_combobox(ui.cbx_resolution,tr("Invalid"));
	}

	// 保持寬高比
	ui.cbx_KeepAspect->setChecked(config.bKeepAspect);

	// 码率
	if(config.vBitrate > 1.f)
	{
		QString vb = QString::number(config.vBitrate/1000,'f',0) + QString("Kbps");
		_g_add_to_combobox(ui.cbx_video_bitrate,vb);
	}
	else
	{
		_g_add_to_combobox(ui.cbx_video_bitrate,tr("Invalid"));
	}
	

	// 帧率
	if(config.vFPS > 1.f)
	{
		QString fps = QString::number(config.vFPS);
		_g_add_to_combobox(ui.cbx_fps,fps);
		
	}
	else
	{
		_g_add_to_combobox(ui.cbx_fps,tr("Invalid"));
	}

	// 音频码率
	if(config.aBitrate > 1.f)
	{
		QString ab = QString::number(config.aBitrate/1000,'f',format == "3GP" ? 1 : 0) + QString("Kbps");
		_g_add_to_combobox(ui.cbx_audio_bitrate,ab);
	}
	else
	{
		ui.cbx_audio_bitrate->addItem(tr("Invalid"));
		_g_add_to_combobox(ui.cbx_audio_bitrate,tr("Invalid"));
	}

	// 声道
	if(config.aChannels > 0)
	{
		QString c = QString::number(config.aChannels);
		_g_add_to_combobox(ui.cbx_channels,c);

	}
	else
	{
		_g_add_to_combobox(ui.cbx_channels,tr("Invalid"));
	}

	// 采样率
	if(config.aSamplings > 0)
	{
		QString sample = QString::number(config.aSamplings);
		_g_add_to_combobox(ui.cbx_sample,sample);
	}
	else
	{
		_g_add_to_combobox(ui.cbx_sample,tr("Invalid"));
	}
}

//自定義格式改變
void xParameterEx_ui::slot_formatChanged( const QString & text )
{
	// 预定义的参数
	PREDEFINED_FORMAT pre_defined_config;
	if(!xConfigManager::Instance().GetPreDefinedFormatParam(text, pre_defined_config))
		return;

	// 视频编码方式
	ui.cbx_encode->clear();
	
	if (!pre_defined_config.vFormats.isEmpty())
		_g_add_to_combobox(ui.cbx_encode,pre_defined_config.vFormats);
	else
		_g_add_to_combobox(ui.cbx_encode,tr("Invalid"));

	// 分辨率
	ui.cbx_resolution->clear();
	if (!pre_defined_config.vResolutions.isEmpty())
		_g_add_to_combobox(ui.cbx_resolution,pre_defined_config.vResolutions);
	else
		_g_add_to_combobox(ui.cbx_resolution,tr("Invalid"));

	// 码率
	ui.cbx_video_bitrate->clear();
	QStringList v_bitrates_kbps;
	for(int idx = 0; idx < pre_defined_config.vBitrates.size(); idx++)
	{
		if(pre_defined_config.vBitrates[idx].toInt())
		{
			v_bitrates_kbps <<  QString::number((pre_defined_config.vBitrates[idx].toInt() / 1000),'f',0) + QString("Kbps");
		}
	}
	if (!v_bitrates_kbps.isEmpty())
		_g_add_to_combobox(ui.cbx_video_bitrate,v_bitrates_kbps);
	else
		_g_add_to_combobox(ui.cbx_video_bitrate,tr("Invalid"));


	// 帧率
	ui.cbx_fps->clear();
	if (!pre_defined_config.vFPS.isEmpty())
		_g_add_to_combobox(ui.cbx_fps,pre_defined_config.vFPS);
	else
		_g_add_to_combobox(ui.cbx_fps,tr("Invalid"));


	// 音频码率
	ui.cbx_audio_bitrate->clear();
	QStringList a_bitrates_kbps;
	for(int idx = 0; idx < pre_defined_config.aBitrates.size(); idx++)
	{
		if(pre_defined_config.aBitrates[idx].toInt())
		{
			a_bitrates_kbps <<  QString::number((pre_defined_config.aBitrates[idx].toFloat() / 1000),'f',text == QString("3GP") ? 1 : 0) + QString("Kbps");
		}
	}

	if (!a_bitrates_kbps.isEmpty())
		_g_add_to_combobox(ui.cbx_audio_bitrate,a_bitrates_kbps);

	else
		_g_add_to_combobox(ui.cbx_audio_bitrate,tr("Invalid"));

	// 声道
	ui.cbx_channels->clear();
	if (!pre_defined_config.aChannels.isEmpty())
		_g_add_to_combobox(ui.cbx_channels,pre_defined_config.aChannels);
	else
		_g_add_to_combobox(ui.cbx_channels,tr("Invalid"));

	// 采样率
	ui.cbx_sample->clear();
	if (!pre_defined_config.aSamplings.isEmpty())
		_g_add_to_combobox(ui.cbx_sample,pre_defined_config.aSamplings);	
	else
		_g_add_to_combobox(ui.cbx_sample,tr("Invalid"));

}

void xParameterEx_ui::slot_keepAspect_Changed(int state)
{
	if (mbIsCustomized)
	{
		DEVICE_CONFIGURE config;
		if(!xConfigManager::Instance().GetDefaultParam(mDevice, mModel, config))
		{
			return;
		}	
		config.bKeepAspect = ui.cbx_KeepAspect->isChecked();
		xConfigManager::Instance().ModifyCustumDevice(config);
	}
}
void xParameterEx_ui::slot_audioVideo_Changed(const QString& str)
{
	if (mbIsCustomized && !str.isEmpty())
	{
		DEVICE_CONFIGURE config;
		if(!xConfigManager::Instance().GetDefaultParam(mDevice, mModel, config))
		{
			return;
		}	

		if (QObject::sender() == (QObject*)ui.cbx_format)
		{
			config.fileFormats.clear();
			config.fileFormats << str;
		}
		else if(QObject::sender() == (QObject*)ui.cbx_encode)
		{
			config.vFormats.clear();
			config.aFormats.clear();

			QStringList strList = str.split("+");
			if (strList.size() ==  2)
			{
				config.vFormats << strList[0];
				config.aFormats << strList[1];
			}
			else if(strList.size() == 1)
			{
				if (g_isVideoFormat(strList[0]))
					config.vFormats << strList[0];
				else
					config.aFormats << strList[0];
			}
		}
		else if(QObject::sender() == (QObject*)ui.cbx_resolution)
		{
			config.vResolutions.clear();
			config.vResolutions << str;
		}
		else if(QObject::sender() == (QObject*)ui.cbx_video_bitrate)
		{
			QString strTemp = str;
			QString vbitrate = strTemp.remove(QString("Kbps"));
			config.vBitrate  = int(vbitrate.toFloat() * 1000);
		}
		else if(QObject::sender() == (QObject*)ui.cbx_fps)
		{
			config.vFPS = str.toUInt();
		}
		else if(QObject::sender() == (QObject*)ui.cbx_sample)
		{
			config.aSamplings = str.toUInt();
		}
		else if(QObject::sender() == (QObject*)ui.cbx_audio_bitrate)
		{
			QString strTemp = str;
			QString abitrate = strTemp.remove(QString("Kbps"));
			config.aBitrate = abitrate.toDouble() * 1000;
		}
		else if(QObject::sender() == (QObject*)ui.cbx_channels)
		{	
			config.aChannels = str.toUInt();
		}

		xConfigManager::Instance().ModifyCustumDevice(config);
	}
}

// void xParameterEx_ui::get_output_config(OUTPUT_CONFIG& config)
// {
// 	// 容器类型
// 	config.type = ui.cbx_format->currentText();
// 
// 	// 视频编码
// 	config.video_type = mVideoEncode;//ui.cbx_encode->currentText();
// 
// 	// 分辨率
// 	config.video_resolution = ui.cbx_resolution->currentText();
// 
// 	// 视频码率
// 	config.video_bitrate = ui.cbx_video_bitrate->currentText();
// 
// 	// 视频帧率
// 	config.video_fps = ui.cbx_fps->currentText();
// 
// 	// 音频编码
// 	config.audio_type = mAudioEncode;//ui.cbx_audio_encode->currentText();
// 
// 	// 音频码率
// 	config.audio_bitrate = ui.cbx_audio_bitrate->currentText();
// 
// 	// 声道数量
// 	config.audio_channel = ui.cbx_channels->currentText();
// 
// 	// 采样
// 	config.audio_sampling = ui.cbx_sample->currentText();
// 	
// 	config.audio_fileSize = 0;
// }

void xParameterEx_ui::get_transcode_config(TRANSCODE_CONFIG& config)
{
	// 容器类型
	config.ContainerType = ContainerFromString(ui.cbx_format->currentText().toStdWString().c_str());

	// 视频编码
	config.VideoCodec.CodecType = VideoEncoderFromString(ui.cbx_encode->currentText().toStdWString().c_str());

	// 分辨率
	if(config.VideoCodec.CodecType == _V_OTHER)
	{
		config.VideoCodec.width = 0;
		config.VideoCodec.height = 0;
		
	}
	else
	{
		QStringList widthHeight = ui.cbx_resolution->currentText().split('x');
		Q_ASSERT(widthHeight.size() == 2);
		config.VideoCodec.width = widthHeight[0].toInt();
		config.VideoCodec.height = widthHeight[1].toInt();
	}

	config.KeepAspectRatio = ui.cbx_KeepAspect->isChecked();

	// 视频码率
	QString vbitrate = ui.cbx_video_bitrate->currentText().remove(QString("Kbps"));
	config.VideoCodec.bitrate = int(vbitrate.toFloat() * 1000);

	// 视频帧率
	config.VideoCodec.framerate = ui.cbx_fps->currentText().toInt();

	// 音频编码
	config.AudioCodec.CodecType = AudioEncoderFromString(ui.cbx_encode->currentText().toStdWString().c_str());

	// 音频码率
	QString abitrate = ui.cbx_audio_bitrate->currentText().remove(QString("Kbps"));
	config.AudioCodec.bitrate = int(abitrate.toFloat() * 1000);

	// 声道数量
	config.AudioCodec.channel = ui.cbx_channels->currentText().toInt();

	// 采样
	config.AudioCodec.samplingrate = ui.cbx_sample->currentText().toInt();

}

void xParameterEx_ui::slot_engine_statusChanged(E_ENGINE_STATES s)
{
	if (s == ENGINE_STATES_TRANSCODING || s == ENGINE_STATES_PAUSE)
	{
		ui.cbx_KeepAspect->setEnabled(false);
		ui.btnCustum->setEnabled(false);
		ui.cbx_format->setEnabled(false);
		ui.cbx_resolution->setEnabled(false);
		ui.cbx_fps->setEnabled(false);
		ui.cbx_video_bitrate->setEnabled(false);
		ui.cbx_encode->setEnabled(false);
		ui.cbx_sample->setEnabled(false);
		ui.cbx_audio_bitrate->setEnabled(false);
		ui.cbx_channels->setEnabled(false);
		//ui.cbx_audio_encode->setEnabled(false);
	}
	else
	{
		ui.btnCustum->setEnabled(true);
		if (ui.btnCustum->isChecked())
		{
			ui.cbx_KeepAspect->setEnabled(true);
			ui.cbx_format->setEnabled(true);
			ui.cbx_encode->setEnabled(true);
			ui.cbx_resolution->setEnabled(true);
			ui.cbx_fps->setEnabled(true);
			ui.cbx_video_bitrate->setEnabled(true);
			//ui.cbx_audio_encode->setEnabled(true);
			ui.cbx_audio_bitrate->setEnabled(true);
			ui.cbx_channels->setEnabled(true);
			ui.cbx_sample->setEnabled(true);
		}
	}
}

void xParameterEx_ui::save_to_setting(QSettings& set)
{
	set.beginGroup("paramex");
	set.setValue("custum", ui.btnCustum->isChecked() ? "1" : "0");
	set.setValue("format", ui.cbx_format->currentText());
	
	set.setValue("video", ui.cbx_encode->currentText());
	set.setValue("resolution", ui.cbx_resolution->currentText());
	set.setValue("fps", ui.cbx_fps->currentText());
	set.setValue("vb",ui.cbx_video_bitrate->currentText());
	//set.setValue("audio", ui.cbx_audio_encode->currentText());
	set.setValue("ab",ui.cbx_audio_bitrate->currentText());
	set.setValue("channel",ui.cbx_channels->currentText());
	set.setValue("sample",ui.cbx_sample->currentText());
	set.setValue("aspectlock",ui.cbx_KeepAspect->isChecked() ? "1" : "0");

	set.endGroup();

}

void xParameterEx_ui::load_from_setting(QSettings& set)
{
	set.beginGroup("paramex");
	
	int bCustom = set.value("custum").toInt();
	if (bCustom)
	{
		ui.btnCustum->setChecked(true);	
		
		_g_add_to_combobox(ui.cbx_format, set.value("format").toString());
 		_g_add_to_combobox(ui.cbx_encode, set.value("video").toString());
		_g_add_to_combobox(ui.cbx_resolution, set.value("resolution").toString());
		_g_add_to_combobox(ui.cbx_fps, set.value("fps").toString());
		_g_add_to_combobox(ui.cbx_video_bitrate, set.value("vb").toString());
		//_g_add_to_combobox(ui.cbx_audio_encode, set.value("audio").toString());
		_g_add_to_combobox(ui.cbx_audio_bitrate, set.value("ab").toString());
		_g_add_to_combobox(ui.cbx_channels, set.value("channel").toString());
		_g_add_to_combobox(ui.cbx_sample, set.value("sample").toString());

		ui.cbx_KeepAspect->setChecked(set.value("aspectlock","1") == "1" );
	}
	else
	{
		ui.btnCustum->setChecked(false);
		ui.cbx_KeepAspect->setChecked(true);
	}
	set.endGroup();
}
