#include "xTableModel.h"
#include "MessageDefine.h"
#include "xTableView_ui.h"
#include <QStandardItem>

#define  ROW_ITEM_COUNT	4
xTableModel::xTableModel(QObject *parent)
	: QStandardItemModel(parent)
{
	mpItemOperator = dynamic_cast<xIItemOperator*>(parent);
}

xTableModel::~xTableModel()
{
	
}


void xTableModel::setActive(bool bActive)
{
	if (bActive)
	{
		connect(mTaskManager.get(),SIGNAL(sig_AsynFresh(int)), this, SLOT(slot_AsynFresh(int)), Qt::QueuedConnection);
		connect(mTaskManager.get(),SIGNAL(sig_AsynItemStatusChanged(int,int)), this, SLOT(slot_AsynItemStatusChanged(int,int)), Qt::QueuedConnection);
	}
	else
	{
		disconnect(mTaskManager.get(),SIGNAL(sig_AsynFresh(int)), this, 0);
	}
}

void xTableModel::slot_addTask( int task_id )
{
	int row = 0;
	
	xTaskItemPtr taskItem = mTaskManager ? mTaskManager->get_item_by_task_id(task_id) : xTaskItemPtr();

	// 找到一个空行放置
	if (task_id != -1)
	{
		for(row = 0; row < rowCount(); row++)
		{
			QStandardItem* item_status = item(row,0);
			if(item_status->data().toInt() == -1)
			{
				item_status->setData(task_id);
				item_status->setToolTip(tr("Wait"));

				if(taskItem)
				{
					QStandardItem* item_url = item(row,1);
					item_url->setToolTip(taskItem->get_url());
				}
				
				mpItemOperator->fresh(task_id);
				return;
			}
		}
	}
	
	if(task_id == -1 || row == rowCount())
	{
		//Append
		QList<QStandardItem *> items_list;

		//status
		QStandardItem* item_status = new QStandardItem();
		item_status->setSelectable(true);
		item_status->setEditable(false);
		item_status->setData(QVariant(task_id));
		if (task_id!=-1)
			item_status->setToolTip(tr("Wait"));

		//url
		QStandardItem* item_url = new QStandardItem();
		item_url->setSelectable(true);
		item_url->setEditable(false);
		if(taskItem)
			item_url->setToolTip(taskItem->get_url());

		//progress
		QStandardItem* item_progress = new QStandardItem();
		item_progress->setSelectable(true);
		item_progress->setEditable(false);
		if(taskItem)
			item_progress->setToolTip(taskItem->get_url());

		//time leave
		QStandardItem* item_time = new QStandardItem();
		item_time->setSelectable(true);
		item_time->setEditable(false);
		if(taskItem)
			item_time->setToolTip(taskItem->get_url());

		items_list << item_status << item_url << item_progress << item_time;
	
		appendRow(items_list);
	}
}

void xTableModel::slot_removeTask(int task_id)
{
	for (int i = 0; i < rowCount(); i++)
	{
		QStandardItem* it = item(i,0);
		if (it && it->data().toInt() == task_id)
		{
			QList<QStandardItem*> items = takeRow(i);
			for (int j = 0; j < items.size(); j++)
			{
				delete items[j];
			}
			break;
		}
	}
}

void xTableModel::slot_taskCountChanged(int count)
{
	if (count == 0)
	{
		// clear rows
		for (int i = rowCount() - 1; i >= 0 ; i--)
		{
			QList<QStandardItem*> items = takeRow(i);
			for (int j = 0; j < items.size(); j++)
			{
				delete items[j];
			}
		}
	}
	
	if (rowCount() < DEFAULT_ROW)
	{
		int toAdd = DEFAULT_ROW - rowCount();
		for (int i = 0; i < toAdd; i++)
			slot_addTask(-1);
	}
}

void xTableModel::slot_moveTask(int task_id, int dst_index)
{
	for (int i = 0; i < rowCount(); i++)
	{
		QStandardItem* it = item(i,0);
		if (it->data().toInt() == task_id)
		{ 
			QList<QStandardItem *> items = takeRow(i);
			if (!items.isEmpty())
			{		
				insertRow(dst_index,items);
				mpItemOperator->set_selected(task_id);

				mpItemOperator->show_item(task_id);
				break;
			}
		}
	}
}

void xTableModel::setTaskMananger(xTaskManagerUIPtr manager)
{
	mTaskManager = manager;

	//connect(mTaskManager,SIGNAL(sig_AsynFresh(int)), this, SLOT(slot_AsynFresh(int)), Qt::QueuedConnection);
	connect(mTaskManager.get(),SIGNAL(sig_addTask(int)), this, SLOT(slot_addTask(int)));
	connect(mTaskManager.get(),SIGNAL(sig_removeTask(int)), this, SLOT(slot_removeTask(int)));
	connect(mTaskManager.get(),SIGNAL(sig_moveTask(int,int)), this, SLOT(slot_moveTask(int,int)));
	connect(mTaskManager.get(),SIGNAL(sig_item_count_changed(int)), this, SLOT(slot_taskCountChanged(int)));
}

void xTableModel::slot_AsynFresh(int taskId)
{
	mpItemOperator->fresh(taskId);
}

void xTableModel::slot_AsynItemStatusChanged(int id, int s)
{
	mpItemOperator->item_status_changed(id, (E_ITME_STATES)s);
}