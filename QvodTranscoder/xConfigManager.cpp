#include "StdAfx.h"
#include "xConfigManager.h"
#include <QFile>
#include <QtXML\qdom.h>
#include <QtXML/QDomNodeList>
#include "commonfunction.h"
#include "xApp.h"


const unsigned int g_key2 = 16;
xConfigManager& xConfigManager::Instance()
{
	static xConfigManager config;
	return config;
}
xConfigManager::xConfigManager(void)
{
	load_xml();

	LoadCustomDevice();
}

xConfigManager::~xConfigManager(void)
{

}

bool xConfigManager::load_xml()
{
	QFile file(QvodApplication::g_CurrentDirectory+"configure.xml");
	file.open(QIODevice::ReadOnly);

	QByteArray bytes = file.readAll();
	
	QByteArray dec_bytes(bytes.size(), 0);
	Dec2((const BYTE*)bytes.data(), (BYTE*)dec_bytes.data(),bytes.size(),g_key2);
	

	QDomDocument doc;
	bool bOK = doc.setContent(dec_bytes);
 	if (!bOK)
 		return false;

	QDomElement e = doc.documentElement();
	QString  strVersion = e.attribute("version",QString("1"));

	// parse predefined formats
	QDomNodeList format_list = e.elementsByTagName("format");
	for(int i = 0; i < format_list.count(); i++)
	{
		QDomElement format_elem = format_list.at(i).toElement();
		PREDEFINED_FORMAT format;
		format.name =  format_elem.attribute("name");

		QStringList formatlist = format_elem.attribute("encode").split(',',QString::SkipEmptyParts);//¾Ž´a·½Ê½½MºÏ
		format.vFormats = formatlist;
		format.vResolutions = format_elem.attribute("resolutions").split(',',QString::SkipEmptyParts);
		format.vFPS = format_elem.attribute("fps").split(',',QString::SkipEmptyParts);
		format.vBitrates = format_elem.attribute("vbitrate").split(',',QString::SkipEmptyParts);
	
		format.aBitrates = format_elem.attribute("abitrate").split(',',QString::SkipEmptyParts);
		format.aSamplings = format_elem.attribute("samplerate").split(',',QString::SkipEmptyParts);
		format.aChannels = format_elem.attribute("channels").split(',',QString::SkipEmptyParts);
		mPredefinedFormats.push_back(format);
		
	}

	// parse device
	int id_temp = 0;
	QDomNodeList devices_list = e.elementsByTagName("device");
	for(int i = 0; i < devices_list.count(); i++)
	{
		QDomElement device = devices_list.at(i).toElement();
		QString device_name = device.attribute("name");
		
		QDomNodeList model_list = device.elementsByTagName("model");
		for(int j = 0; j < model_list.count(); j++)
		{
			QDomElement model = model_list.at(j).toElement();

			DEVICE_CONFIGURE config;
			config.device_id = i;
			config.model_id = j;
			config.device = device_name;
			config.model = model.attribute("name");
			config.fileFormats =  model.attribute("formats").split(',',QString::SkipEmptyParts);
			config.vFormats = model.attribute("video").split(',',QString::SkipEmptyParts);
			config.aFormats = model.attribute("audio").split(',',QString::SkipEmptyParts);
			config.vResolutions =  model.attribute("resolutions").split(',',QString::SkipEmptyParts);
			config.vFPS  = model.attribute("fps").toInt();
			config.vBitrate = model.attribute("vbitrate").toFloat();
			config.aBitrate = model.attribute("abitrate").toFloat();
			config.aSamplings = model.attribute("samplerate").toInt();
			config.aChannels = model.attribute("channels").toInt();

			mConfigList << config;
		}
	}

	return true;
}

 QStringList xConfigManager::GetDevices() const
 {
 	QStringList list;
 	DEVICE_CONFIGURE_LIST::const_iterator i;
 	for (i = mConfigList.constBegin(); i != mConfigList.constEnd(); ++i)
 	{
 		if (!list.contains(i->device))
 			list << i->device;
 	}
 	return list;
 }
 
 QStringList xConfigManager::GetModel(QString device) const
 {
 	QStringList list;
 	DEVICE_CONFIGURE_LIST::const_iterator i;
 	for (i = mConfigList.constBegin(); i != mConfigList.constEnd(); ++i)
 	{
 		if (i->device == device)
 		{
 			list << i->model;
 		}
 	}
 	return list;
 }

 QStringList xConfigManager::GetModelByDevicesID(int device_id) const
 {
	 QStringList list;
	 DEVICE_CONFIGURE_LIST::const_iterator i;
	 for (i = mConfigList.constBegin(); i != mConfigList.constEnd(); ++i)
	 {
		 if (i->device_id == device_id)
		 {
			 list << i->model;
		 }
	 }
	 return list;
 }

bool xConfigManager::GetDefaultParam(int device_id, int model_id, DEVICE_CONFIGURE& config) const
{
	std::list<DEVICE_CONFIGURE> lists = mConfigList.toStdList();
 	DEVICE_CONFIGURE_LIST::const_iterator i;
 	for (i = mConfigList.constBegin(); i != mConfigList.constEnd(); ++i)
 	{
 		if (i->device_id == device_id && i->model_id == model_id)
 		{
 			config = *i;
 			return true;
 		}
 	}
	return false;
}

bool xConfigManager::GetPreDefinedFormatParam(QString format, PREDEFINED_FORMAT& format_param) const
{
	PREDEFINED_FORMAT_LIST::const_iterator i;
	for (i = mPredefinedFormats.begin(); i != mPredefinedFormats.end(); ++i)
	{
		if (i->name == format)
		{
			format_param = *i;
			return true;
		}
	}
	return false;
}

QStringList xConfigManager::GetPreDefinedFormats() const
{
	QStringList list;
	PREDEFINED_FORMAT_LIST::const_iterator i;
	for (i = mPredefinedFormats.begin(); i != mPredefinedFormats.end(); ++i)
	{
		list << i->name;
	}
	return list;
}

int xConfigManager::GetCustomizedDeviceID()
{
	DEVICE_CONFIGURE_LIST::iterator i;
	for (i = mConfigList.begin(); i != mConfigList.end(); ++i)
	{
		if(i->is_customized)
		{
			return i->device_id;
		}
	}
	return -1;
}

void xConfigManager::ModifyCustumDevice(DEVICE_CONFIGURE& configure)
{
	DEVICE_CONFIGURE_LIST::iterator i;
	for (i = mConfigList.begin(); i != mConfigList.end(); ++i)
	{
		if(i->is_customized && i->model_id == configure.model_id)
		{
			i->fileFormats = configure.fileFormats;
			i->bKeepAspect = configure.bKeepAspect;
			i->aBitrate = configure.aBitrate;
			i->aChannels = configure.aChannels;
			i->aFormats = configure.aFormats;
			i->aSamplings = configure.aSamplings;
			i->vBitrate = configure.vBitrate;
			i->vFormats = configure.vFormats;
			i->vFPS = configure.vFPS;
			i->vResolutions = configure.vResolutions;
			break;
		}
	}
}

void xConfigManager::SaveCustumDevice()
{
	
	QSettings settings(QvodApplication::g_CurrentDirectory + "config.ini", QSettings::IniFormat);

	int j = 0;
	for (DEVICE_CONFIGURE_LIST::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
	{
		if (i->is_customized)
		{
			settings.beginGroup(QString::number(j++));
			settings.setValue("format", i->fileFormats.size() ? i->fileFormats[0] : "");
			settings.setValue("video", i->vFormats.size() ? i->vFormats[0] : "");
			settings.setValue("widthxheight", i->vResolutions.size() ? i->vResolutions[0] : "");
			settings.setValue("fps", i->vFPS);
			settings.setValue("vbitrate", QString::number(int(i->vBitrate)));
			settings.setValue("audio", i->aFormats.size() ? i->aFormats[0] : "");
			settings.setValue("abitrate", QString::number(int(i->aBitrate)));
			settings.setValue("channel", i->aChannels);
			settings.setValue("sample", i->aSamplings);
			settings.endGroup();
		}
	}
}

void xConfigManager::LoadCustomDevice()
{
	QStringList devices = GetDevices();
	QSettings settings(QvodApplication::g_CurrentDirectory + "config.ini", QSettings::IniFormat);
// 	int c = settings.value("count").toInt();
// 	if (!c)
// 	{
// 		for (int i = 0; i < 4; i++)
// 		{
// 			DEVICE_CONFIGURE config;
// 			config.device_id = devices.size();
// 			config.model_id = i;
// 			config.is_customized = true;
// 			config.device = QString("Customized device");
// 			config.model =  QString("My device") + QString::number(i+1);
// 			mConfigList.push_back(config);
// 		}
// 	}
// 	else
// 	{
	
		for (int i = 0 ; i < 4; i++)
		{
			DEVICE_CONFIGURE config;
			settings.beginGroup(QString::number(i));
			config.device_id = devices.size();
			config.model_id = i;
			config.is_customized = true;
			config.device = "Customized device";
			config.model = QString("My device") + QString::number(i+1);

			config.fileFormats << settings.value("format").toString();
			if (config.fileFormats[0].isEmpty())
			{
				config.fileFormats[0] = "MP4";
			}
			config.vFormats << settings.value("encode").toString();
			if (config.vFormats[0].isEmpty())
			{
				config.vFormats[0] = "MPEG4";
			}
			config.vResolutions << settings.value("widthxheight").toString();
			if (config.vResolutions[0].isEmpty())
			{
				config.vResolutions[0] = "640x480";
			}

			config.vFPS  = settings.value("fps").toInt();
			if (!config.vFPS)
				config.vFPS  = 20;

			config.vBitrate = settings.value("vbitrate").toInt();
			if (!config.vBitrate)
				config.vBitrate  = 768000;	

			config.aFormats << settings.value("audio").toString();
			if (config.aFormats[0].isEmpty())
			{
				config.aFormats[0] = "AAC";
			}

			config.aBitrate = settings.value("abitrate").toInt();
			if (!config.aBitrate)
				config.aBitrate  = 128000;

			config.aChannels = settings.value("channel").toInt();
			if (!config.aChannels)
				config.aChannels  = 2;

			config.aSamplings = settings.value("sample").toInt();
			if (!config.aSamplings)
				config.aSamplings  = 44100;

			settings.endGroup();

			mConfigList.push_back(config);
		}
	//}
}