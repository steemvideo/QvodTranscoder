#ifndef XPARAMETER_H
#define XPARAMETER_H

#include <QWidget>
#include "ui_xParameter.h"
#include "xlangchangelisten.h"

class xParameter_ui : public QWidget,public xLangChangeListen
{
	Q_OBJECT

public:
	xParameter_ui(QWidget *parent = 0);
	~xParameter_ui();

	void load_param();
	void save_to_setting(QSettings& set);
	void load_from_setting(QSettings& set);
	virtual void retranslateUi();
signals:
	void sig_expand(bool);
	void sig_deviceModel_Changed(int device,int model);
	void sig_multiTask(bool);
	//void sig_accelerateOpen(bool);

private:
	Ui::xParameterClass ui;
	bool				mbExpand;	

	void	init_ui();
	void	doParseSample();

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private slots:
	//void on_cbx_acce_stateChanged(int);
	void on_cbxMultiTask_stateChanged(int);
	void on_btnExpand_clicked();
	void paintEvent(QPaintEvent* event);
	void slot_onDeviceChanged(int);
	void slot_onModelChanged(int);

	void slot_engine_statusChanged(E_ENGINE_STATES s);
};

#endif // XPARAMETER_H
