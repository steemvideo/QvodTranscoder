#include "xTableModel.h"
#include "xTaskManagerUI.h"

#include "xTableView_ui.h"
#include "xTableItemDelegate.h"
#include "xTranscoder.h"
#include "xMediaInfo_ui.h"
#include "xFunction.h"
#include "xMessageBox_ui.h"

xTableView_ui::xTableView_ui(QWidget *parent)
	: QTableView(parent)
	, mMouseHoverRow(-1)
{
	setAcceptDrops(true);
	mModel = new xTableModel(this);

	mModel->setColumnCount(4);
	mModel->setHeaderData(0,Qt::Horizontal,tr("Status"));
	mModel->setHeaderData(1,Qt::Horizontal,tr("File name"));
	mModel->setHeaderData(2,Qt::Horizontal,tr("Progress"));
	mModel->setHeaderData(3,Qt::Horizontal,tr("Remaining time"));

	setModel(mModel);
	setItemDelegate(new xTableItemDelegate(this));	

 	setSelectionMode(QAbstractItemView::ExtendedSelection);
 	setSelectionBehavior(QAbstractItemView::SelectRows);
	
	verticalHeader()->hide();
	horizontalHeader()->setResizeMode(0,QHeaderView::Fixed);
	horizontalHeader()->setResizeMode(3,QHeaderView::Fixed);
	horizontalHeader()->setClickable(false);
	horizontalHeader()->setMovable(false);
	horizontalHeader()->setMinimumSectionSize(50);
	horizontalHeader()->setStretchLastSection(true);
	
	setColumnWidth(0,50);
	setColumnWidth(1,300);
	setColumnWidth(2,150);
	setColumnWidth(3,70);

	setMouseTracking(true);

	init_menu();

	connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
		this, SLOT(slot_selectChanged(const QItemSelection&,const QItemSelection&)));


	for (int i = 0 ; i < DEFAULT_ROW; i++)
		mModel->slot_addTask(-1);
}

xTableView_ui::~xTableView_ui()
{

}

void xTableView_ui::init_menu()
{
	mMenu = new QMenu(this);
	connect(mMenu,SIGNAL(aboutToShow()), this, SLOT(slot_menuAboutToShow()));

	mActPause = mMenu->addAction(tr("Pause"));
	connect(mActPause,SIGNAL(triggered(bool)),this,SLOT(slot_pause(bool)));

	mActStop = mMenu->addAction(tr("Stop"));
	connect(mActStop,SIGNAL(triggered(bool)),this,SLOT(slot_stop(bool)));

	mActPlay = mMenu->addAction(tr("Play"));
	connect(mActPlay,SIGNAL(triggered(bool)),this,SLOT(slot_play(bool)));
	
	mActPlayOutput = mMenu->addAction(tr("Play output file"));
	connect(mActPlayOutput,SIGNAL(triggered(bool)),this,SLOT(slot_play_output(bool)));
	
	mActOpenSourceDir = mMenu->addAction(tr("Open source folder"));
	connect(mActOpenSourceDir,SIGNAL(triggered(bool)),this,SLOT(slot_open_source_dir(bool)));

	mActTry = mMenu->addAction(tr("Try"));
	connect(mActTry, SIGNAL(triggered(bool)), this, SLOT(slot_try(bool)));

	mActAddSubTitle = mMenu->addAction(tr("Add subtitle"));
	connect(mActAddSubTitle,SIGNAL(triggered(bool)),this,SLOT(slot_add_subtitle(bool)));

	mActRemoveSubTitle = mMenu->addAction(tr("Remove subtitle"));
	connect(mActRemoveSubTitle,SIGNAL(triggered(bool)),this,SLOT(slot_remove_subtitle(bool)));

	mActRemoveFile = mMenu->addAction(tr("Remove"));
	connect(mActRemoveFile,SIGNAL(triggered(bool)),this,SLOT(slot_remove_file(bool)));

	mActClearList = mMenu->addAction(tr("Clear task list"));
	connect(mActClearList,SIGNAL(triggered(bool)),this,SLOT(slot_clear_list(bool)));

	mActMediaInfo = mMenu->addAction(tr("Media information"));
	connect(mActMediaInfo,SIGNAL(triggered(bool)),this,SLOT(slot_media_info(bool)));
}
void xTableView_ui::keyPressEvent ( QKeyEvent * event )
{
	if (event->key() == Qt::Key_Delete || 
		event->key() == Qt::Key_X)
	{
		remove_selected_items();
	}
}

bool xTableView_ui::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		QModelIndex index = indexAt(helpEvent->pos());
		QStandardItem * pItem_new = mModel->item(index.row(), index.column());
		if (pItem_new) {
			QToolTip::showText(helpEvent->globalPos(), pItem_new->toolTip());
		} else {
			QToolTip::hideText();
			event->ignore();
		}

		return true;
	}
	return QTableView::event(event);
}

QList<int> xTableView_ui::selected_items()
{
	QList<int> items;
	QModelIndexList list = selectionModel()->selectedRows();
	for (int i = 0; i < list.size(); i++)
	{
		QStandardItem* it = mModel->item(list[i].row(), list[i].column());
		if(it && it->data().toInt() != -1)
			items << it->data().toInt();
	}
	return items;
}

void xTableView_ui::show_item(int task_id)
{
	for (int i = 0; i < mModel->rowCount(); i++)
	{
		if (mModel->item(i,0)->data().toInt() == task_id)
		{
			showRow(i);
			break;
		}
	}
	
}

void xTableView_ui::set_selected(int task_id)
{
	for (int i = 0; i < mModel->rowCount(); i++)
	{
		if (mModel->item(i,0)->data().toInt() == task_id)
		{
			selectRow(i);
			break;
		}
	}
	emit sig_select_changed(selected_items().size());
}

void xTableView_ui::item_status_changed(int task_id, E_ITME_STATES s)
{
	for (int i = 0; i < mModel->rowCount(); i++)
	{
		if (mModel->item(i,0)->data().toInt() == task_id)
		{
			switch(s)
			{
			case ITEM_STATES_WAIT:
				mModel->item(i,0)->setToolTip(tr("Wait"));
				break;
			case ITEM_STATES_COMPLETE:
				mModel->item(i,0)->setToolTip(tr("Complete"));
				break;
			case ITEM_STATES_PAUSE:
				mModel->item(i,0)->setToolTip(tr("Pause"));
				break;
			case ITEM_STATES_STOP:
				mModel->item(i,0)->setToolTip(tr("Stop"));
				break;
			case ITEM_STATES_ERROR:
				mModel->item(i,0)->setToolTip(tr("Error"));
				break;
			case ITEM_STATES_TRANSCODING:
				mModel->item(i,0)->setToolTip(tr("Transcoding"));
				break;
			}
			break;
		}
	}
}

void xTableView_ui::slot_selectChanged(const QItemSelection& s,const QItemSelection& ds)
{
	int count = selected_items().size();
	emit sig_select_changed(count);
}

void xTableView_ui::mouseMoveEvent ( QMouseEvent * event )
{
	int row_old = mMouseHoverRow;
	QModelIndex index = indexAt(event->pos());
	if (index.isValid())
	{
		mMouseHoverRow = index.row();
	}
	else
	{
		mMouseHoverRow = -1;
	}

	if (mMouseHoverRow != row_old)
	{
		for (int i = 0; i < 4; i++)
		{
			QStandardItem * pItem_new = mModel->item(mMouseHoverRow, i);
			if(pItem_new)
				update(pItem_new->index());

			QStandardItem * pItem_old = mModel->item(row_old, i);
			if (pItem_old)
				update(pItem_old->index());
		}
	}

	QTableView::mouseMoveEvent(event);
}

void xTableView_ui::mousePressEvent (QMouseEvent * event )
{
	QTableView::mousePressEvent(event);
}

void xTableView_ui::leaveEvent( QEvent * event )
{
	for (int i = 0; i < 4; i++)
	{
		QStandardItem * pItem_new = mModel->item(mMouseHoverRow, i);
		if(pItem_new)
			update(pItem_new->index());
	}
	
	mMouseHoverRow = -1;

	QTableView::leaveEvent(event);
}

void xTableView_ui::contextMenuEvent ( QContextMenuEvent * e )
{
	if (mpTaskManager->item_count() < 1)
	{
		return;
	}
	mPosMenu = e->pos();
	mMenu->exec(e->globalPos());
}

void xTableView_ui::slot_try(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = mModel->item(index.row(), 0)->data().toInt();
	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
	{
		emit sig_try(ptr->get_url());
	}
}

void xTableView_ui::slot_play(bool)
{
// 	QModelIndex index = indexAt(mPosMenu);
// 	int taskId = mModel->item(index.row(), 0)->data().toInt();
// 	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
// 	if (ptr)
// 	{
// 		QString str = ptr->get_url();
// 		if (!str.isEmpty())
// 		{
// 			g_play_file(str);
// 		}
// 	}

	QList<int> items = selected_items();
	for (int i = 0; i < items.size(); i++)
	{
		xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(items[i]);
	 	if (ptr)
	 	{
	 		QString str = ptr->get_url();
	 		if (!str.isEmpty())
	 		{
	 			g_play_file(str);
	 		}
	 	}
	}
}

void xTableView_ui::slot_play_output(bool)
{
// 	QModelIndex index = indexAt(mPosMenu);
// 	int taskId = mModel->item(index.row(), 0)->data().toInt();
// 	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
// 	if (ptr && ptr->get_states() == ITEM_STATES_COMPLETE)
// 	{
// 		QString str = ptr->get_dest_url();
// 		if (!str.isEmpty())
// 		{
// 			g_play_file(str);
// 		}
// 	}
	QList<int> items = selected_items();
	for (int i = 0; i < items.size(); i++)
	{
		xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(items[i]);
		if (ptr && ptr->get_states() == ITEM_STATES_COMPLETE)
		{
			QString str = ptr->get_dest_url();
			if (!str.isEmpty())
			{
				g_play_file(str);
			}
		}
	}

}

void xTableView_ui::slot_open_source_dir(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = mModel->item(index.row(), 0)->data().toInt();
	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
	{
// 		QFileInfo info(ptr->get_url());
// 		QString str = info.absolutePath();
		g_open_url(ptr->get_url());
	}
}

void xTableView_ui::slot_add_subtitle(bool)	
{
	QString fileName = QFileDialog::getOpenFileName(this,tr("Select subtitle file"),"",tr("subtitle file(*.srt;*.ass;*.ssa;*.sub;*.idx;*.smi;*.txt)"));
	QModelIndex index = indexAt(mPosMenu);
	int taskId = mModel->item(index.row(), 0)->data().toInt();
	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
		ptr->set_title_file_url(fileName);
}

void xTableView_ui::slot_remove_subtitle(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = mModel->item(index.row(), 0)->data().toInt();
	
	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
		ptr->set_title_file_url("");

}

void xTableView_ui::slot_remove_file(bool)
{
	remove_selected_items();
}

void xTableView_ui::slot_clear_list(bool)
{
	if (ENGINE_STATES_STOP != mpTaskManager->get_engine_states())
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The file is being transcoded,Are you sure to delete it?"));
		box.SetButtonVisabled(true,true);
		if(QDialog::Rejected == box.exec())
			return;
	}
	mpTaskManager->clear_all();
}

void xTableView_ui::slot_media_info(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = mModel->item(index.row(), 0)->data().toInt();
	if (taskId != -1)
		emit sig_show_media_info(taskId);
}

void xTableView_ui::fresh(int task_id)
{
	for (int r = 0; r < mModel->rowCount(); r++)
	{
		if (mModel->item(r,0)->data().toInt() == task_id)
		{
			for (int i = 0; i < 4; i++)
			{
				QStandardItem * pItem_new = mModel->item(r, i);
				if(pItem_new)
				{
					update(pItem_new->index());
				}
			}
			break;
		}
	}
}

void xTableView_ui::dragEnterEvent ( QDragEnterEvent * event )
{
	QDragMoveEvent* dragMove =  static_cast<QDragMoveEvent *>(event);
	dragMoveEvent(dragMove);
}

void xTableView_ui::dragMoveEvent ( QDragMoveEvent * event )
{
	if(event->mimeData()->urls().size())
	{
		event->acceptProposedAction();
	}
}

QStringList xTableView_ui::getDropPath(QList<QUrl>& urls)
{
	QStringList ret;
 	foreach(QUrl u, urls)
 	{
 		QDir d(u.toLocalFile());
 		if (d.exists())
 		{
 			QStringList filePath;
			QStringList filter;
			filter << "*.*";
 			filePath << d.entryList(filter, QDir::Files);
 
 			foreach(QString s, filePath)
 			{
 				ret << d.absolutePath() + "/" + s;
 			}
 		}
 		else
 		{
 			ret << u.toLocalFile();
 		}
 		
 	}
	return ret;
}

void xTableView_ui::retranslateUi()
{
	mModel->setHeaderData(0,Qt::Horizontal,tr("Status"));
	mModel->setHeaderData(1,Qt::Horizontal,tr("File name"));
	mModel->setHeaderData(2,Qt::Horizontal,tr("Progress"));
	mModel->setHeaderData(3,Qt::Horizontal,tr("Remaining time"));

	mActPause->setText(tr("Pause"));
	mActStop->setText(tr("Stop"));
	mActPlay->setText(tr("Play"));
	mActPlayOutput->setText(tr("Play output file"));
	mActOpenSourceDir->setText(tr("Open source folder"));
	mActAddSubTitle->setText(tr("Add subtitle"));
	mActRemoveSubTitle->setText(tr("Remove subtitle"));
	mActRemoveFile->setText(tr("Remove"));
	mActClearList->setText(tr("Clear task list"));
	mActMediaInfo->setText(tr("Media information"));
	mActTry->setText(tr("Try"));
}

void xTableView_ui::dropEvent ( QDropEvent * event )
{
	bool bErrorFile = false;
	QStringList files = getDropPath(event->mimeData()->urls());
	for (int i = 0; i < files.size(); i++)
	{
		if(E_ERROR_FILE_UNSUPPORTS == mpTaskManager->add_item(files[i]))
			bErrorFile = true;
	}
	if (bErrorFile)
	{	
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The adding file contains non-video format files!"));
		box.SetButtonVisabled(true,false);
		box.exec();
	}
}

void xTableView_ui::slot_menuAboutToShow()
{
	mActPause->setEnabled(false);
	mActStop->setEnabled(false);
	mActPlay->setEnabled(false);
	mActPlayOutput->setEnabled(false);
	mActOpenSourceDir->setEnabled(false);
	mActAddSubTitle->setEnabled(false);
	mActRemoveSubTitle->setEnabled(false);
	mActRemoveFile->setEnabled(false);
	mActClearList->setEnabled(false);
	mActMediaInfo->setEnabled(false);
	mActTry->setEnabled(false);

	

	if (mpTaskManager->item_count() > 0)
	{
		mActClearList->setEnabled(true);
	}
	
	QList<int> selectedItems = selected_items();

	// 命中？
	QModelIndex index = indexAt(mPosMenu);
	QStandardItem* it = mModel->item(index.row(), 0);
	if (it)
	{
		if (mpTaskManager->get_engine_states() == ENGINE_STATES_STOP && selectedItems.size() == 1)
		{
			mActTry->setEnabled(true);
		}

		xTaskItemPtr itemHit = mpTaskManager->get_item_by_task_id(it->data().toInt());
		if (itemHit)
		{
			E_ITME_STATES s = mpTaskManager->get_item_status(itemHit->get_task_id());

			// Play
			mActPlay->setEnabled(true);

			mActOpenSourceDir->setEnabled(selectedItems.size() > 1 ? false : true);

			mActRemoveFile->setEnabled(true);
		

			if (s == ITEM_STATES_TRANSCODING)
			{
				mActPause->setEnabled(true);
				mActPause->setText(tr("Pause"));
				mActStop->setEnabled(true);
			}

			if (s == ITEM_STATES_PAUSE)
			{
				mActPause->setEnabled(true);
				mActPause->setText(tr("Continue"));
				mActStop->setEnabled(true);

				bool bMultiTran = mpTaskManager->is_multi_task();
				int cout = mpTaskManager->get_count_by_status(ITEM_STATES_TRANSCODING);
				if (cout >= (bMultiTran ? MAX_TRANSCODING_COUNT : 1))
				{
					mActPause->setEnabled(false);
				}	
			}

			if (s == ITEM_STATES_WAIT || s == ITEM_STATES_STOP)
			{
				mActAddSubTitle->setEnabled(selectedItems.size() > 1 ? false : true);

				// remove title
				QString url_title = itemHit->get_title_file_url();
				if (!url_title.isEmpty())
					mActRemoveSubTitle->setEnabled(true);
			}

			mActMediaInfo->setEnabled(selectedItems.size() > 1 ? false : true);
		}
	}

	// 播放输出文件
	mActPlayOutput->setEnabled(false);
	for (int i = 0; i < selectedItems.size(); i++)
	{
		xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(selectedItems[i]);
		if (ptr)
		{
			// Play output file
			if (ptr->get_states() == ITEM_STATES_COMPLETE)
			{
				mActPlayOutput->setEnabled(true);
				break;
			}
		}
	}
	
}

void xTableView_ui::slot_pause(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	QStandardItem* item = mModel->item(index.row(), 0);
	if (item)
	{
		int task_id = item->data().toInt();
		E_ITME_STATES s  = mpTaskManager->get_item_status(task_id);

		if(s == ITEM_STATES_TRANSCODING)
			mpTaskManager->pause_item(task_id);
		else if(s == ITEM_STATES_PAUSE)
			mpTaskManager->continue_item(task_id);
	}
}

void xTableView_ui::slot_stop(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	QStandardItem* item = mModel->item(index.row(), 0);
	if (item)
	{
		mpTaskManager->stop_item(item->data().toInt());
	}
}

void xTableView_ui::set_task_manager(xTaskManagerUIPtr manager)
{
	mpTaskManager = manager;
	mModel->setTaskMananger(manager);
}

void xTableView_ui::setActive(bool bCurrent)
{
	mModel->setActive(bCurrent);
}

void xTableView_ui::de_selected_all()
{
	selectionModel()->clear();
}

void xTableView_ui::remove_selected_items()
{
	QList<int>  items = selected_items();
	if (items.isEmpty())
		return;

	bool bNeedNotify = false;
	for (int i = items.size() - 1; i >= 0; i--)
	{
		E_ITME_STATES s = mpTaskManager->get_item_status(items[i]);
		bNeedNotify = (s == ITEM_STATES_PAUSE ||  s == ITEM_STATES_TRANSCODING);
		if (bNeedNotify)
			break;
	}
	if (bNeedNotify)
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The file is being transcoded,Are you sure to delete it?"));
		box.SetButtonVisabled(true,true);
		if(QDialog::Rejected == box.exec())
			return;
	}

	for (int i = items.size() - 1; i >= 0; i--)
		mpTaskManager->remove_item(items[i]);
}