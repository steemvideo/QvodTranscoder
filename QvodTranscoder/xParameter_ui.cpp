#include "xParameter_ui.h"
#include "xConfigManager.h"
#include "xParseSample_ui.h"
#include "xFunction.h"

#define INTEL_VERSION

xParameter_ui::xParameter_ui(QWidget *parent)
	: QWidget(parent)
	, mbExpand(false)
{
	ui.setupUi(this);

	init_ui();
}

xParameter_ui::~xParameter_ui()
{


}

void xParameter_ui::retranslateUi()
{
	ui.retranslateUi(this);
	if(mbExpand)
		ui.btnExpand->setToolTip(tr("Close details"));
	else
		ui.btnExpand->setToolTip(tr("Open details"));
	load_param();
	
}
void xParameter_ui::init_ui()
{
	QListView* view = (QListView*)ui.cbx_device->view();
	view->setMinimumWidth(200);
	view->setMinimumHeight(400);

	view = (QListView*)ui.cbx_model->view();
	view->setMinimumWidth(200);

	connect(ui.cbx_device,SIGNAL(currentIndexChanged(int)),this,SLOT(slot_onDeviceChanged(int)));
	connect(ui.cbx_model,SIGNAL(currentIndexChanged(int)),this,SLOT(slot_onModelChanged(int)));

	ui.lblFindDevice->installEventFilter(this);

	ui.btnExpand->setToolTip(tr("Open details"));

#ifndef INTEL_VERSION
	ui.cbx_acce->setEnabled(false);
#endif

}

void xParameter_ui::on_btnExpand_clicked()
{
	mbExpand = !mbExpand;
	if(mbExpand)
		ui.btnExpand->setToolTip(tr("Close details"));
	else
		ui.btnExpand->setToolTip(tr("Open details"));
	emit sig_expand(mbExpand);
}

void xParameter_ui::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect r = rect();
	r.adjust(0,30,-1,-1);
	QPen pen(Qt::gray);
	pen.setWidth(1);
	painter.setPen(pen);

	
 	QLinearGradient gradient(QPoint(0,0),QPoint(0,height()));
 	gradient.setColorAt(0,QColor(255,255,255));
 	gradient.setColorAt(1,QColor(234,234,234));
	QBrush brush(gradient);
	painter.setBrush(brush);

	painter.drawRect(r);
}

void xParameter_ui::slot_onDeviceChanged(int device_id)
{
	ui.cbx_model->blockSignals(true);
	ui.cbx_model->clear();
	ui.cbx_model->blockSignals(false);

	QStringList models = xConfigManager::Instance().GetModelByDevicesID(device_id);
	TranslateQList(models);
	ui.cbx_model->addItems(models);
	{
		//设置高度
		for (int i = 0; i < ui.cbx_model->count();i++)
		{
			ui.cbx_model->model()->setData(ui.cbx_model->model()->index(i, 0), QSize(220, 22), Qt::SizeHintRole);
		}
	}
	
	ui.cbx_model->setCurrentIndex(0);
}

void xParameter_ui::slot_onModelChanged(int model_index)
{
	int device_index = ui.cbx_device->currentIndex();
	emit sig_deviceModel_Changed(device_index, model_index);
}

void xParameter_ui::on_cbxMultiTask_stateChanged(int s)
{
	emit sig_multiTask(s == Qt::Checked);
}

void xParameter_ui::load_param() 
{	
	int curindex = ui.cbx_device->currentIndex();
	if(curindex < 0)
	{
		curindex = 0;
	}
	//clear list
	ui.cbx_device->clear();

	QStringList devices = xConfigManager::Instance().GetDevices();
	TranslateQList(devices);//翻譯成目標版本語言

	ui.cbx_device->addItems(devices);
	ui.cbx_device->setCurrentIndex(curindex);

	{
		//设置高度
		for (int i = 0; i < ui.cbx_device->count();i++)
		{
			ui.cbx_device->model()->setData(ui.cbx_device->model()->index(i, 0), QSize(180, 22), Qt::SizeHintRole);
		}
	}

}

void xParameter_ui::slot_engine_statusChanged(E_ENGINE_STATES s)
{
	if (s == ENGINE_STATES_TRANSCODING || s == ENGINE_STATES_PAUSE)
	{
		ui.cbx_device->setEnabled(false);
		ui.cbx_model->setEnabled(false);
#ifndef INTEL_VERSION
		ui.cbx_acce->setEnabled(false);
#endif
		ui.lblFindDevice->setEnabled(false);
	}
	else
	{
		ui.cbx_device->setEnabled(true);
		ui.cbx_model->setEnabled(true);
#ifndef INTEL_VERSION
		ui.cbx_acce->setEnabled(true);
#endif
		ui.lblFindDevice->setEnabled(true);
	}
}

bool xParameter_ui::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) 
	{
		QMouseEvent *keyEvent = static_cast<QMouseEvent *>(event);
		if (keyEvent->button() == Qt::LeftButton && ui.lblFindDevice->isEnabled())
		{
			doParseSample();
		}
		return true;
	}
	else
	{
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}

void xParameter_ui::doParseSample()
{
	xParseSample_ui	window(this);
	if(QDialog::Rejected == window.exec())
		return;

	int deviceid = xConfigManager::Instance().GetCustomizedDeviceID();
	int modelid = window.getCustomizedModelID();
	
	// 分析成功并切换到“我的设备x”
	ui.cbx_device->setCurrentIndex(deviceid);
	if(modelid != ui.cbx_model->currentIndex())
	{
		ui.cbx_model->setCurrentIndex(modelid);
	}
	else
	{
		slot_onModelChanged(modelid);
	}
}

void xParameter_ui::save_to_setting(QSettings& set)
{
	set.beginGroup("param");
	set.setValue("device", QString::number(ui.cbx_device->currentIndex()));
	set.setValue("model", QString::number(ui.cbx_model->currentIndex()));
	set.endGroup();

}

void xParameter_ui::load_from_setting(QSettings& set)
{
	set.beginGroup("param");
	ui.cbx_device->setCurrentIndex(set.value("device").toInt());
	int pos = ui.cbx_model->currentIndex();
	ui.cbx_model->setCurrentIndex(set.value("model").toInt());
	if (pos == ui.cbx_model->currentIndex())
	{
		slot_onModelChanged(pos);
	}
	set.endGroup();
}


// void xParameter_ui::on_cbx_acce_stateChanged(int s)
// {
// 	emit sig_accelerateOpen(s == Qt::Checked);
// }