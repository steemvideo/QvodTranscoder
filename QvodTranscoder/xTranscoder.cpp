#include "xTranscoder.h"
#include "xTableItemDelegate.h"
#include <QFileDialog>
#include <QStackedLayout>
#include "xMessageBox_ui.h"
#include "xConfigManager.h"
#include "xMediaInfo_ui.h"
#include "xTaskManagerUI.h"
#include "xTester_ui.h"
#include "xFunction.h"
#include "xApp.h"

//#define DEF_TEST
xTranscoder::xTranscoder(ITaskManager* m, Qt::WFlags flags)
	: QWidget(NULL, flags | Qt::FramelessWindowHint)
	, mbMoveable(false)
	, mbStopManual(false)
	, mbShowFullDisk(false)
{	
	setAttribute(Qt::WA_TranslucentBackground);
	mpTaskManager.reset(new xTaskManagerUI(m));

	init_ui();
	
	//load xml
	xConfigManager::Instance();
}

xTranscoder::~xTranscoder()
{
	mpTaskManager.reset();

	write_ini();

	xConfigManager::Instance().SaveCustumDevice();
}

void xTranscoder::init_ui()
{
	ui.setupUi(this);
	
	init_component();

	//load ini
	load_ini();

	connect(QvodApplication::instance(),SIGNAL(sig_DeviceChanged(bool,QString)),this,SLOT(slot_DeviceChanged(bool,QString)));
}

void xTranscoder::load_ini()
{
	QSettings settings("config.ini", QSettings::IniFormat, this);
	mLastDir = settings.value("url").toString();
	
	mpUI->wnd_parameter->load_from_setting(settings);
	mpUI->wnd_parameter_detail->load_from_setting(settings);
	mpUI->wnd_output->load_from_setting(settings);
}
void xTranscoder::write_ini()
{
	QSettings settings("config.ini", QSettings::IniFormat, this);
	settings.setValue("url", mLastDir);
	
	mpUI->wnd_parameter->save_to_setting(settings);
	mpUI->wnd_parameter_detail->save_to_setting(settings);
	mpUI->wnd_output->save_to_setting(settings);
}


void xTranscoder::init_component()
{
	mpUI = new xMain_UI;
	mpUI->setupUi(this);
	mpUI->wnd_table->set_task_manager(mpTaskManager);
	mpUI->wnd_list->set_task_manager(mpTaskManager);
	mpItemOperator = (xIItemOperator*)mpUI->wnd_table;
	mpItemOperator->setActive(true);

	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_engine_states_changed(E_ENGINE_STATES)), this, SLOT(slot_engine_states_changed(E_ENGINE_STATES)));
	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_AsynDiskFull()), this, SLOT(slot_AsynDiskFull()),Qt::QueuedConnection);

	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_engine_states_changed(E_ENGINE_STATES)), mpUI->wnd_parameter_detail, SLOT(slot_engine_statusChanged(E_ENGINE_STATES)));
	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_engine_states_changed(E_ENGINE_STATES)), mpUI->wnd_parameter, SLOT(slot_engine_statusChanged(E_ENGINE_STATES)));
	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_engine_states_changed(E_ENGINE_STATES)), mpUI->wnd_output, SLOT(slot_engine_statusChanged(E_ENGINE_STATES)));
	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_engine_states_changed(E_ENGINE_STATES)), mpUI->wnd_bottom, SLOT(slot_engine_statusChanged(E_ENGINE_STATES)));
	connect((QObject*)mpTaskManager.get(),SIGNAL(sig_item_count_changed(int)), mpUI->wnd_bottom, SLOT(slot_item_changed(int)));

	connect(mpUI->wnd_parameter,SIGNAL(sig_deviceModel_Changed(int,int)),  mpUI->wnd_parameter_detail,SLOT(slot_deviceModel_Changed(int,int)));
	connect(mpUI->wnd_parameter,SIGNAL(sig_expand(bool)), this,SLOT(slot_showDetail(bool)));
	connect(mpUI->wnd_parameter,SIGNAL(sig_multiTask(bool)), this,SLOT(slot_multiTask(bool)));
	//connect(mpUI->wnd_parameter,SIGNAL(sig_accelerateOpen(bool)), this,SLOT(slot_accelerateOpen(bool)));
	

	connect(mpUI->wnd_bottom,SIGNAL(sig_start()), this, SLOT(on_btnBegin_clicked()));
	connect(mpUI->wnd_bottom,SIGNAL(sig_stop()), this, SLOT(on_btnStop_clicked()));

	connect(mpUI->wnd_tools,SIGNAL(sig_import()), this,SLOT(slot_btnOpen_clicked()));
	connect(mpUI->wnd_tools,SIGNAL(sig_remove()), this,SLOT(slot_btnRemove_clicked()));
	connect(mpUI->wnd_tools,SIGNAL(sig_up()), this,SLOT(slot_btnUp_clicked()));
	connect(mpUI->wnd_tools,SIGNAL(sig_down()), this,SLOT(slot_btnDown_clicked()));
	connect(mpUI->wnd_tools,SIGNAL(sig_modelChanged(bool)), this,SLOT(slot_btnViewMode_clicked(bool)));

	connect(mpUI->wnd_table, SIGNAL(sig_select_changed(int)),  mpUI->wnd_tools, SLOT(slot_select_changed(int)));
	connect(mpUI->wnd_table,SIGNAL(sig_show_media_info(int)), this,SLOT(slot_show_media_info(int)));	
	connect(mpUI->wnd_list,SIGNAL(sig_show_media_info(int)), this,SLOT(slot_show_media_info(int)));	
	connect(mpUI->wnd_table,SIGNAL(sig_try(QString)), this,SLOT(slot_try(QString)));
	connect(mpUI->wnd_list,SIGNAL(sig_try(QString)), this,SLOT(slot_try(QString)));

	connect(mpUI->wnd_title,SIGNAL(sig_close()), this,SLOT(slot_close()));
	connect(mpUI->wnd_title,SIGNAL(sig_min()), this,SLOT(slot_min()));
}

void xTranscoder::slot_btnOpen_clicked()
{
	QStringList files = QFileDialog::getOpenFileNames( this,tr("Select one or more files to open"),mLastDir,tr("Media files (*.*)"));
	bool bError = false;
	for (int i = 0; i < files.size(); i++)
	{
		if(E_ERROR_FILE_UNSUPPORTS == mpTaskManager->add_item(files[i]))
		{
			bError = true;
		}
	}
	
	if (bError)
	{	
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The adding file contains non-video format files!"));
		box.SetButtonVisabled(true,false);
		box.exec();
	}

	if (files.size())
		mLastDir = QFileInfo(files[0]).absolutePath();
}

QString xTranscoder::get_correct_url()
{
	QString url = mpUI->wnd_output->getOutputDir();
	
	if (url.isEmpty())
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("Please select valid output folder"));
		box.SetButtonVisabled(true,false);
		box.exec();
		return QString();
	}
	QDir d(url);
	if(!d.exists())
	{
		bool bOK = d.mkpath(url);
		if(!bOK)
		{
			xMessageBox box(this);
			box.SetTitle(tr("Tips"));
			box.SetText(tr("Invalid save path!"));
			box.SetButtonVisabled(true,false);
			box.exec();
			return QString();
		}
	}
	// 杜绝光驱等路径
	{
		//磁盘是可写？
		ULARGE_INTEGER   lpFreeBytesAvailableToCaller; 
		ULARGE_INTEGER   lpTotalNumberOfBytes; 
		ULARGE_INTEGER   lpTotalNumberOfFreeBytes; 
		QString disk = url.left(3);
		if(!GetDiskFreeSpaceEx(disk.toStdWString().c_str(),   &lpFreeBytesAvailableToCaller,   &lpTotalNumberOfBytes,   &lpTotalNumberOfFreeBytes))
		{
			xMessageBox box(this);
			box.SetTitle(tr("Tips"));
			box.SetText(tr("Invalid save path!"));
			box.SetButtonVisabled(true,false);
			box.exec();
			return QString();
		}
	}
	if(url.right(1) != "/" && url.right(1) != "\\")
	{
		url += "\\";
	}
	return url;
}

void xTranscoder::on_btnBegin_clicked()
{
	QString url = get_correct_url();
	if(url.isEmpty())
	{
		return;
	}

	if (mpTaskManager->get_engine_states() == ENGINE_STATES_PAUSE)
	{
		mpTaskManager->start();
		return;
	}
	else if(mpTaskManager->get_engine_states() == ENGINE_STATES_TRANSCODING)
	{
		mpTaskManager->pause();
		return;
	}

	if (mpTaskManager->item_count() < 1)
	{
		slot_btnOpen_clicked();
		if(mpTaskManager->item_count() < 1)
		{
			return;
		}
	}

	bool bReset = false;
	if(mpTaskManager->get_count_by_status(ITEM_STATES_COMPLETE) == mpTaskManager->item_count())
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("Task completed, do you want to restart?"));
		box.SetButtonVisabled(true,true);
		if(QDialog::Accepted != box.exec())
			return;	

		bReset = true;
	}

	// reset all status
	if(bReset)
		mpTaskManager->reset_all_items(true);
	else
		mpTaskManager->reset_all_items(false);
		
	// set directory
	mpTaskManager->set_directory(url);

	// set configure
	TRANSCODE_CONFIG config;
	get_configure(config);

	if(!mpTaskManager->set_item_configure(config))
	{
		return;
	}

	// 启动
	mpTaskManager->start();

	mbStopManual = false;
	mbShowFullDisk = false;
}

void xTranscoder::get_configure(TRANSCODE_CONFIG& config)
{
	mpUI->wnd_parameter_detail->get_transcode_config(config);
}

void xTranscoder::slot_show_media_info(int taskid)
{
	xTaskItemPtr it = mpTaskManager->get_item_by_task_id(taskid);
	if (!it)
	{
		return;
	}

	boost::shared_ptr<Qvod_DetailMediaInfo>	pMediaInfo = mpTaskManager->get_src_media_info(taskid);
	if (pMediaInfo)
	{
		TRANSCODE_CONFIG config;
		mpUI->wnd_parameter_detail->get_transcode_config(config);
		mpTaskManager->adjust_configure(taskid,config);

		xMediaInfo_ui ui(this);
		ui.set_media_info(pMediaInfo,config);
		ui.exec();
	}
}
void xTranscoder::on_btnStop_clicked()
{
	mbStopManual =  true;
	mpTaskManager->stop();
}

void xTranscoder::slot_btnRemove_clicked()
{
	mpItemOperator->remove_selected_items();
}

void xTranscoder::mousePressEvent(QMouseEvent *event)  
{  
	if(event->button() == Qt::LeftButton)  
	{  
		mDragPosition = event->globalPos() - frameGeometry().topLeft();    
		mbMoveable = true;  
		
		event->accept();  
	}  
}  

void xTranscoder::mouseMoveEvent(QMouseEvent *event)  
{  
	if(event->buttons() & Qt::LeftButton && mbMoveable)  
	{  
		move(event->globalPos() - mDragPosition);  
		event->accept();  
	}  
}  

void xTranscoder::mouseReleaseEvent(QMouseEvent* event)  
{  
	if(mbMoveable)  
	{  
		mbMoveable = false;  
	}  
}  

void xTranscoder::slot_btnDown_clicked()
{
	QList<int> selected_items = mpItemOperator->selected_items();
	if(selected_items.size())
	{
		int idx = mpTaskManager->get_item_index(selected_items[0]);
		mpTaskManager->move_item(selected_items[0], idx+1);
	}
}

void xTranscoder::slot_btnUp_clicked()
{
	QList<int> selected_items = mpItemOperator->selected_items();
	if(selected_items.size())
	{
		int idx = mpTaskManager->get_item_index(selected_items[0]);
		mpTaskManager->move_item(selected_items[0], idx-1);
	}
}

void xTranscoder::slot_showDetail(bool bExpaned)
{
	if (bExpaned)
	{
		resize(620, 567);
		mpUI->wnd_parameter_detail->show();
	}
	else
	{
		mpUI->wnd_parameter_detail->hide();
		adjustSize();
		resize(620, 445);
		
	}
}

void xTranscoder::slot_multiTask(bool bMultiTask)
{
	mpTaskManager->set_multi_task(bMultiTask);
}

// void xTranscoder::slot_accelerateOpen(bool bOpen)
// {
// 	mpTaskManager->set_accelerateOpened(bOpen);
// }

void xTranscoder::slot_btnViewMode_clicked(bool bTableMode)
{
	QList<int> selectedItems = mpItemOperator->selected_items();

	mpItemOperator->de_selected_all();
	mpItemOperator->setActive(false);
	mpUI->stacked_layout->setCurrentIndex(bTableMode ? 0 : 1);
	
	if (bTableMode)
	{
		disconnect(mpUI->wnd_list, SIGNAL(sig_select_changed(int)),  mpUI->wnd_tools, 0);
		connect(mpUI->wnd_table, SIGNAL(sig_select_changed(int)),  mpUI->wnd_tools, SLOT(slot_select_changed(int)));
		mpItemOperator = (xIItemOperator*)mpUI->wnd_table;
	}
	else
	{
		disconnect(mpUI->wnd_table, SIGNAL(sig_select_changed(int)),  mpUI->wnd_tools, 0);
		connect(mpUI->wnd_list, SIGNAL(sig_select_changed(int)),  mpUI->wnd_tools, SLOT(slot_select_changed(int)));
		mpItemOperator = (xIItemOperator*)mpUI->wnd_list;
	}


	mpItemOperator->setActive(true);
	mpItemOperator->de_selected_all();

	if (mpItemOperator == (xIItemOperator*)mpUI->wnd_table)
		mpUI->wnd_table->setSelectionMode(QAbstractItemView::MultiSelection);//防止多选不上
	
	for(int i = 0; i < selectedItems.size(); i++)
		mpItemOperator->set_selected(selectedItems[i]);

	if (mpItemOperator == (xIItemOperator*)mpUI->wnd_table)
		mpUI->wnd_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void xTranscoder::slot_close()
{
	E_ENGINE_STATES s = mpTaskManager->get_engine_states();
	if (s == ENGINE_STATES_TRANSCODING || s == ENGINE_STATES_PAUSE)
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("Transcoding, are you sure to quit?"));
		box.SetButtonVisabled(true,true);
		if(QDialog::Accepted != box.exec())
			return;	
	}

	mpTaskManager->stop();
	close();
}

void xTranscoder::slot_min()
{
	//setWindowState(Qt::WindowMinimized);
	showMinimized();
}

void xTranscoder::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::HighQualityAntialiasing);

	QPen pen(QColor(172,172,172));
	pen.setWidth(1);
	painter.setPen(pen);

 	QLinearGradient gradient(QPoint(0,0),QPoint(0,height()));

	gradient.setColorAt(0,QColor(251,251,251));
	gradient.setColorAt(0.1,QColor(232,232,232));
	gradient.setColorAt(0.2,QColor(233,233,233));
	gradient.setColorAt(0.8,QColor(225,225,225));
 	gradient.setColorAt(1,QColor(215,215,215));
	QBrush brush(gradient);
	painter.setBrush(brush);

	QRect rect_bg = rect();
	rect_bg.adjust(0,0,-1,-1);
	painter.drawRoundedRect(rect_bg, 5,5);
	
	QPen pen2(QColor(255,255,255));
	pen2.setWidth(1);

	painter.setPen(pen2);
	rect_bg.adjust(1,1,-1,-1);
	painter.drawRoundedRect(rect_bg, 5,5);

	QPen p(QColor(193,193,193));
	p.setWidth(1);
	painter.setPen(p);
	painter.drawLine(QPoint(0,30), QPoint(width(),30));
}

//QString xTranscoder::get_output_format() const
//{
// 	OUTPUT_CONFIG config;
// 	mpUI->wnd_parameter_detail->get_output_config(config);
// 	return config.type;
//}

void xTranscoder::slot_engine_states_changed(E_ENGINE_STATES s)
{
	if (s == ENGINE_STATES_STOP 
		&& mpUI->wnd_bottom->is_shutdown_checked()
		&& !mbStopManual
		&& mpTaskManager->get_count_by_status(ITEM_STATES_COMPLETE) > 0)
	{
		xMessageBox box(this);
		box.SetTitle(tr("Shutdown notice"));
		box.SetButtonText(0,tr("Shut down immediately"));
		box.SetButtonVisabled(true,true);

		box.SetText(tr("Transcoding completed. Your computer will be shut down in %d seconds, you can:"), "%d", 30);
		
		if(QDialog::Accepted != box.exec())
			return;	

		shutdown();
	}
	if (s == ENGINE_STATES_TRANSCODING)
	{
		mbStopManual = false;
	}
}

void xTranscoder::shutdown()
{
	HANDLE hToken;  
	TOKEN_PRIVILEGES tkp;  
	OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);  
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);  
	tkp.PrivilegeCount = 1;  
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES)NULL, 0);  
	ExitWindowsEx(EWX_SHUTDOWN , 0);  
}

void xTranscoder::slot_try(QString file_url)
{
	qDebug() << "# BEGIN TRY";
	QString url = get_correct_url();
	if(url.isEmpty())
	{
		return;
	}

	if (mpTaskManager->item_count() < 1)
	{
		slot_btnOpen_clicked();
	}

	qDebug() << "# item_count";
// 	xTaskItemPtr item = mpTaskManager->get_item_by_index(0);
// 	if (!item)
// 	{
// 		return;
// 	}

	// set directory
	boost::scoped_ptr<xTaskManagerUI> pManagerUI(new xTaskManagerUI(g_get_task_manager()));
	pManagerUI->set_directory(url);

	// set configure
	TRANSCODE_CONFIG config;
	get_configure(config);
	if (config.ContainerType == _E_CON_OTHER)
	{
		return;
	}

	pManagerUI->set_item_configure(config);
//	pManagerUI->set_output_format(get_output_format());
	qDebug() << "# set_output_format";

	boost::scoped_ptr<xTester_ui> pTesterUI(new xTester_ui(pManagerUI.get(),this));
	if(E_ERROR_FILE_UNSUPPORTS == pManagerUI->add_item(/*item->get_url()*/file_url))
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The adding file contains non-video format files!"));
		box.SetButtonVisabled(true,false);
		box.exec();
		return;
	}

	qDebug() << "# start";
	pManagerUI->start(true);
	if(QDialog::Accepted == pTesterUI->exec())
	{
		xTaskItemPtr it = pManagerUI->get_item_by_index(0);
		g_open_url(it->get_dest_url());
	}
	
	pManagerUI->clear_all();
	qDebug() << "# END TRY";
}

void xTranscoder::load_urls(QStringList& urls)
{
	bool bError = false;
	for (int i = 0; i < urls.size(); i++)
	{
		if(E_ERROR_FILE_UNSUPPORTS == mpTaskManager->add_item(urls[i]))
			bError = true;
	}
	if (bError)
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The adding file contains non-video format files!"));
		box.SetButtonVisabled(true,false);
		box.exec();
		return;
	}
}

// bool xTranscoder::winEvent ( MSG * msg, long * result )
// {
// 	
// // 	else if(msg->message == WM_LANG_CHANGE)
// // 	{
// // 		int lang = msg->lParam;
// // 		QvodApplication::instance()->set_current_lang(lang);
// // 	}
// 	return QWidget::winEvent(msg, result);
// }

void xTranscoder::slot_AsynDiskFull()
{
	if (!mbShowFullDisk)
	{
		mbShowFullDisk = true;

		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("Disk is full. Make sure you have enough space and try again."));
		box.SetButtonVisabled(true,false);
		box.exec();

		mpTaskManager->stop();
	}
}

void xTranscoder::slot_DeviceChanged(bool bAdd,QString strDriver)
{
	if (!bAdd)
	{
		strDriver = strDriver.left(2);

		QString url = mpUI->wnd_output->getOutputDir();
		url = url.left(2);

		if(!url.compare(strDriver,Qt::CaseInsensitive))
		{
			if (mpTaskManager->get_engine_states() == ENGINE_STATES_TRANSCODING)
			{
				mpTaskManager->stop();
				
				xMessageBox box(this);
				box.SetTitle(tr("Tips"));
				box.SetText(tr("External device has removed.Please insert and try again!"));
				box.SetButtonVisabled(true,false);
				box.exec();
			}
		}
	}
}