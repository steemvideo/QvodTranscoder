#ifndef XPARAMETEREX_UI_H
#define XPARAMETEREX_UI_H

#include <QWidget>
#include "ui_xParameterEx.h"
#include "xlangchangelisten.h"

// struct OUTPUT_CONFIG
// {
// 	QString type;
// 	QString video_type;
// 	QString video_bitrate;
// 	QString video_fps;
// 	QString video_resolution;
// 	QString audio_type;
// 	QString audio_bitrate;
// 	QString audio_channel;
// 	QString audio_sampling;
// 	int		audio_fileSize;
// 
// };
struct DEVICE_CONFIGURE;
class xParameterEx_ui : public QWidget, public xLangChangeListen
{
	Q_OBJECT

public:
	xParameterEx_ui(QWidget *parent = 0);
	~xParameterEx_ui();

	void get_transcode_config(TRANSCODE_CONFIG& config);
	//void get_output_config(OUTPUT_CONFIG& config);

	void save_to_setting(QSettings& set);
	void load_from_setting(QSettings& set);

	virtual void retranslateUi();
public slots:
	void slot_deviceModel_Changed(int device,int model);	//设备、型号改变
	void slot_formatChanged( const QString & text );			//
	void slot_engine_statusChanged(E_ENGINE_STATES);
	void slot_audioVideo_Changed(const QString&);
	void slot_keepAspect_Changed(int);

protected:
	void paintEvent(QPaintEvent* event);
	void init_ui();
	void set_force_config(DEVICE_CONFIGURE&);	
	void set_Default();
private:
	Ui::xParameterExClass ui;
	int mDevice;
	int mModel;
	bool mbIsCustomized;
private slots:
	void on_checkBox_stateChanged(int);
};

#endif // XPARAMETEREX_UI_H
