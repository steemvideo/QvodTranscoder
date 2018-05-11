#pragma once
#include <QObject>
#include "xlangchangelisten.h"
#include <string>


struct DEVICE_CONFIGURE
{
	DEVICE_CONFIGURE()
	{
		bKeepAspect =true;
		is_customized = false;
		device_id = model_id = 0;
		vFPS = aSamplings = aChannels = 0;
		vBitrate = aBitrate = 0.f;
	}
	int			device_id;
	int			model_id;
	bool	is_customized;
	QString device;
	QString model;

	QStringList fileFormats;
	QStringList vFormats;
	QStringList aFormats;
	QStringList	vResolutions;
	int			vFPS;
	float		vBitrate;
	bool		bKeepAspect;
	float		aBitrate;
	int			aSamplings;
	int			aChannels;
};
typedef QList<DEVICE_CONFIGURE> DEVICE_CONFIGURE_LIST;
struct PREDEFINED_FORMAT
{
	QString name;
	QStringList vFormats;
	QStringList vResolutions;
	QStringList vFPS;
	QStringList vBitrates;
	QStringList aFormats;
	QStringList aBitrates;
	QStringList aSamplings;
	QStringList aChannels;
};
typedef std::vector<PREDEFINED_FORMAT> PREDEFINED_FORMAT_LIST;


class xConfigManager:public QObject 
{
public:
	static xConfigManager& Instance();
	QStringList GetDevices() const;				//未翻譯字符串
	QStringList GetModel(QString device) const;	//未翻譯字符串
	QStringList GetModelByDevicesID(int) const; //用设备ID找型号字符串

	bool		GetDefaultParam(int device_index, int model_index, DEVICE_CONFIGURE& config) const;

	// 預置格式
	QStringList GetPreDefinedFormats() const;
	bool		GetPreDefinedFormatParam(QString format, PREDEFINED_FORMAT& format_config) const;

	// 修改/增加自定义设备
	int			GetCustomizedDeviceID();
	void		ModifyCustumDevice(DEVICE_CONFIGURE& configure);
	void		SaveCustumDevice();

protected:
	xConfigManager(void);
	~xConfigManager(void);

	bool load_xml();
	void LoadCustomDevice();
private:
	DEVICE_CONFIGURE_LIST	mConfigList;
	PREDEFINED_FORMAT_LIST	mPredefinedFormats;
};

