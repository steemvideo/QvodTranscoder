#ifndef XTRANSCODER_H
#define XTRANSCODER_H

#include <QtGui/QWidget>
#include "ui_xTranscoder.h"
#include "xMain_ui.h"

class xTaskManagerUI;
class QvodApplication;
class xTranscoder : public QWidget
{
	Q_OBJECT

	friend class QvodApplication;
public:
	xTranscoder(ITaskManager* m, Qt::WFlags flags = 0);
	~xTranscoder();

	void	get_configure(TRANSCODE_CONFIG& config);
	void	load_urls(QStringList& urls);
	xTaskManagerUIPtr get_taskManager() const { return mpTaskManager;}

public slots:
	void on_btnBegin_clicked();
	void on_btnStop_clicked();

	void slot_btnOpen_clicked();
	void slot_btnRemove_clicked();
	void slot_btnDown_clicked();
	void slot_btnUp_clicked();
	void slot_btnViewMode_clicked(bool);
	void slot_showDetail(bool);
	void slot_close();
	void slot_min();
	void slot_multiTask(bool bMultiTask);
	//void slot_accelerateOpen(bool bOpen);

	void slot_show_media_info(int taskid);
	void slot_engine_states_changed(E_ENGINE_STATES);

	void slot_try(QString);
	void slot_AsynDiskFull();
	void slot_DeviceChanged(bool,QString);
private:
	void init_ui();
	void init_component();
	void load_ini();
	void write_ini();
	void shutdown();
//	QString get_output_format() const;
	QString get_correct_url();
protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);
	//bool winEvent ( MSG * message, long * result );

protected:
	xTaskManagerUIPtr mpTaskManager;
	xIItemOperator*	mpItemOperator;
	
protected:
	QPoint			mDragPosition;
	bool			mbMoveable;	
	Ui::QvodTranscoderClass ui;
	xMain_UI		*mpUI;
	QString			mLastDir;
	bool			mbStopManual;
	bool			mbShowFullDisk;
};

#endif // XTRANSCODER_H
