#pragma once

#define DEFAULT_ROW	9
#include "xTaskManagerUI.h"
#include "xIItemOperator.h"

class xTableModel : public QStandardItemModel
{
	Q_OBJECT
public:
	xTableModel(QObject *parent);
	~xTableModel();	
	void setTaskMananger(xTaskManagerUIPtr manager);
	void setActive(bool);
public slots:
	void	slot_addTask(int taskId);
	void	slot_removeTask(int taskId);
	void	slot_moveTask(int taskId, int dst_index);
	void	slot_taskCountChanged(int count);
	void	slot_AsynFresh(int taskId);
	void	slot_AsynItemStatusChanged(int task_id, int s);
private:
	xTaskManagerUIPtr		mTaskManager;
	xIItemOperator*	mpItemOperator;
};
