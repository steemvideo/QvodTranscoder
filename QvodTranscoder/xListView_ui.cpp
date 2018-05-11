#include "xListView_ui.h"
#include "xListItemDelegate.h"
#include "xTaskManagerUI.h"
#include "xFunction.h"
#include "xMessageBox_ui.h"
#include "commonfunction.h"
xListView_ui::xListView_ui(QWidget *parent)
	: QListWidget(parent)
	, mbTernimal(false)
{	
	setItemDelegate(new xListItemDelegate(this));	
	setViewMode( IconMode );
	setIconSize(QSize(140,145));
	setMovement( Static );

	setMouseTracking(true);
	viewport()->setAcceptDrops(true);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	init_menu();
	connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
		this, SLOT(slot_selectChanged(const QItemSelection&,const QItemSelection&)));
	connect(this, SIGNAL(sig_AsynFresh(int)), this, SLOT(slot_AsynFresh(int)));

	
}

xListView_ui::~xListView_ui()
{
	mbTernimal = true;
	if (mGrabThread)
	{
		mConGrabImage.notify_one();
		mGrabThread->join();
		mGrabThread.reset();
	}
}

void xListView_ui::de_selected_all()
{
	selectionModel()->clear();
}


void xListView_ui::setActive(bool bCurrent)
{
	if (bCurrent)
	{
		connect(mpTaskManager.get(),SIGNAL(sig_AsynFresh(int)), this, SLOT(slot_AsynFresh(int)), Qt::QueuedConnection);
		
		if (!mGrabThread)
			mGrabThread.reset(new boost::thread( boost::bind(& xListView_ui::process_get_image, this)));
	}
	else
	{
		disconnect(mpTaskManager.get(), SIGNAL(sig_AsynFresh(int)), this, 0);
	}
	
}

void xListView_ui::set_task_manager(xTaskManagerUIPtr manager)
{
	mpTaskManager = manager;
	
	//connect(mpTaskManager,SIGNAL(sig_AsynFresh(int)), this, SLOT(slot_AsynFresh(int)), Qt::QueuedConnection);
	connect(mpTaskManager.get(),SIGNAL(sig_addTask(int)), this, SLOT(slot_addTask(int)));
	connect(mpTaskManager.get(),SIGNAL(sig_removeTask(int)), this, SLOT(slot_removeTask(int)));
	connect(mpTaskManager.get(),SIGNAL(sig_moveTask(int,int)), this, SLOT(slot_moveTask(int,int)));
	connect(mpTaskManager.get(),SIGNAL(sig_item_count_changed(int)), this, SLOT(slot_taskCountChanged(int)));
}

void xListView_ui::retranslateUi()
{
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

void xListView_ui::fresh(int task_id)
{
	for (int i = 0; i < count(); i++)
	{
		int  t = item(i)->data(0).toInt();
		if (t == task_id)
		{
			QRect r = visualItemRect(item(i));
			QModelIndex index = indexAt(r.center());
			update(index);
			break;
		}
	}
}

void xListView_ui::slot_addTask( int task_id )
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(140,145));
	item->setData(0, QVariant(task_id));
	insertItem(count(), item);

	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(task_id);
	item->setToolTip(ptr->get_url());
	mConGrabImage.notify_one();//抓帧
}

void xListView_ui::slot_removeTask(int task_id)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	int curCount = count();
	for (int i = 0; i < curCount; i++)
	{
		int  t = item(i)->data(0).toInt();
		if (t == task_id)
		{
			QListWidgetItem* it = takeItem(i);
			delete it;
			break;
		}
	}
}

void xListView_ui::slot_taskCountChanged(int c)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	if (c == 0)
	{
		int currentCount = count();
		for (int i = currentCount - 1; i >= 0 ; i--)
		{
			QListWidgetItem* it = takeItem(i);
			delete it;
		}
	}
}

void xListView_ui::slot_moveTask(int task_id, int dst_index)
{
	bool bOK = false;
	boost::recursive_mutex::scoped_lock lock(mMutex);
	for (int i = 0; i < count(); i++)
	{
		int  t = item(i)->data(0).toInt();
		if (t == task_id)
		{
			QListWidgetItem* it1 = takeItem(i);
			insertItem(dst_index, it1);

			scrollToItem(it1);
			bOK = true;
			break;
		}
	}
	if(bOK)
		set_selected(task_id);
}

void xListView_ui::slot_AsynFresh(int task_id)
{
	fresh(task_id);
}

void xListView_ui::init_menu()
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

void xListView_ui::keyPressEvent ( QKeyEvent * event )
{
	if (event->key() == Qt::Key_Delete || 
		event->key() == Qt::Key_X)
	{
		remove_selected_items();
	}
}

QList<int> xListView_ui::selected_items()
{
	QList<int> items;
	QModelIndexList list = selectionModel()->selectedRows();
	for (int i = 0; i < list.size(); i++)
	{
		QListWidgetItem* it = item(list[i].row());
		if(it && it->data(0).toInt() != -1)
			items << it->data(0).toInt();
	}
	return items;
}

void xListView_ui::set_selected(int task_id)
{
	for (int i = 0; i < count(); i++)
	{
		if (item(i)->data(0).toInt() == task_id)
		{
			item(i)->setSelected(true);
			break;
		}
	}

	emit sig_select_changed(selected_items().size());
}


void xListView_ui::slot_selectChanged(const QItemSelection& s,const QItemSelection& ds)
{
	emit sig_select_changed(selected_items().size());
}

bool xListView_ui::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		QModelIndex index = indexAt(helpEvent->pos());
		QListWidgetItem* it = item(index.row());
		if (it) {
			QToolTip::showText(helpEvent->globalPos(), it->toolTip());
		} else {
			QToolTip::hideText();
			event->ignore();
		}

		return true;
	}
	return QListWidget::event(event);
}

void xListView_ui::mouseMoveEvent ( QMouseEvent * event )
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
		QRect r = visualItemRect(item(mMouseHoverRow));
		QModelIndex index = indexAt(r.center());
		update(index);

		QRect r2 = visualItemRect(item(row_old));
		QModelIndex index2 = indexAt(r2.center());
		update(index2);	
	}
	QListWidget::mouseMoveEvent(event);
}

void xListView_ui::mousePressEvent (QMouseEvent * event )
{
	QListWidget::mousePressEvent(event);
}

void xListView_ui::leaveEvent( QEvent * event )
{
	QRect r = visualItemRect(item(mMouseHoverRow));
	QModelIndex index = indexAt(r.center());
	update(index);

	mMouseHoverRow = -1;

	QListWidget::leaveEvent(event);
}

void xListView_ui::contextMenuEvent ( QContextMenuEvent * e )
{
	if (mpTaskManager->item_count() < 1)
	{
		return;
	}
	mPosMenu = e->pos();
	mMenu->exec(e->globalPos());
}

void xListView_ui::slot_play(bool)
{
// 	QModelIndex index = indexAt(mPosMenu);
// 	int taskId = item(index.row())->data(0).toInt();
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

void xListView_ui::show_item(int task_id)
{
	for (int i = 0; i < count(); i++)
	{
		if (item(i)->data(0).toInt() == task_id)
		{
			scrollToItem(item(i));
			break;
		}
	}
}

void xListView_ui::slot_play_output(bool)
{
// 	QModelIndex index = indexAt(mPosMenu);
// 	int taskId = item(index.row())->data(0).toInt();
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

void xListView_ui::slot_open_source_dir(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = item(index.row())->data(0).toInt();
	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
	{
// 		QFileInfo info(ptr->get_url());
// 		QString str = info.absolutePath();
		g_open_url(ptr->get_url());
	}
}

void xListView_ui::slot_add_subtitle(bool)
{
	QString fileName = QFileDialog::getOpenFileName(this,tr("Select subtitle file"),"",tr("subtitle file(*.srt;*.ass;*.ssa;*.sub;*.idx;*.smi;*.txt)"));
	QModelIndex index = indexAt(mPosMenu);
	int taskId = item(index.row())->data(0).toInt();
	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
		ptr->set_title_file_url(fileName);

}

void xListView_ui::slot_remove_subtitle(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = item(index.row())->data(0).toInt();

	xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(taskId);
	if (ptr)
		ptr->set_title_file_url("");

}

void xListView_ui::slot_remove_file(bool)
{
	remove_selected_items();
}

void xListView_ui::slot_clear_list(bool)
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

void xListView_ui::slot_media_info(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	int taskId = item(index.row())->data(0).toInt();
	if (taskId != -1)
		emit sig_show_media_info(taskId);
}

void xListView_ui::dragEnterEvent ( QDragEnterEvent * event )
{
	QDragMoveEvent* dragMove =  static_cast<QDragMoveEvent *>(event);
	dragMoveEvent(dragMove);
}

void xListView_ui::dragMoveEvent ( QDragMoveEvent * event )
{
	if(event->mimeData()->urls().size())
	{
		event->acceptProposedAction();
	}
}

QStringList xListView_ui::getDropPath(QList<QUrl>& urls)
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


void xListView_ui::dropEvent ( QDropEvent * event )
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

void xListView_ui::slot_menuAboutToShow()
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
	
	QList<int>	selectedItems = selected_items();

	// 命中？
	QModelIndex index = indexAt(mPosMenu);
	QListWidgetItem* it = item(index.row());
	if (it)
	{
		if (mpTaskManager->get_engine_states() == ENGINE_STATES_STOP && selectedItems.size() == 1)
		{
			mActTry->setEnabled(true);
		}

		xTaskItemPtr itemHit = mpTaskManager->get_item_by_task_id(it->data(0).toInt());
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

			if (s == ITEM_STATES_WAIT  || s == ITEM_STATES_STOP)
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

void xListView_ui::slot_pause(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	QListWidgetItem* it = item(index.row());
	if (it)
	{
		int task_id = it->data(0).toInt();
		mpTaskManager->get_item_status(task_id);

		if(mpTaskManager->get_item_status(task_id) == ITEM_STATES_TRANSCODING)
			mpTaskManager->pause_item(task_id);
		else if(mpTaskManager->get_item_status(task_id) == ITEM_STATES_PAUSE)
			mpTaskManager->continue_item(task_id);
	}
}

void xListView_ui::slot_stop(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	QListWidgetItem* it = item(index.row());
	if (it)
	{
		mpTaskManager->stop_item(it->data(0).toInt());
	}
}

void xListView_ui::process_get_image()
{
	CoInitialize(NULL);
	while(!mbTernimal)
	{
		int task_id = -1;
		while(task_id == -1)
		{
			boost::recursive_mutex::scoped_lock lock(mMutex);
			for (int i = 0; i < count(); i++)
			{
				xTaskItemPtr it = mpTaskManager->get_item_by_task_id(item(i)->data(0).toInt());
				if (it && !it->has_image())
				{
					task_id = it->get_task_id();
					break;
				}
			}

			if (task_id !=-1)
				break;

			mConGrabImage.wait(mMutex);	
			if (mbTernimal)
				break;
		}

		if (mbTernimal)
			break;

		// get media info
		xTaskItemPtr it = mpTaskManager->get_item_by_task_id(task_id);
		const Qvod_DetailMediaInfo_Ptr mediainfo = g_get_task_manager()->GetMediaInfo(task_id);
		bool		bOK = false;	
		if(mediainfo)
		{
			E_V_CODEC vc = VideoEncoderFromString(QString::fromWCharArray(mediainfo->videoInfo[0]->Codec).toStdWString().c_str());
			if (vc != _V_OTHER)
			{
				CComPtr<IPreview> IPreviewCPtr;
				g_get_task_manager()->CreatePreviewTask(task_id, IPreviewCPtr);

				REFERENCE_TIME pos_second = mediainfo->nDuration / (float)1000 / 2;

				if (IPreviewCPtr)
				{
					IPreviewCPtr->Play();
					REFERENCE_TIME t = pos_second * 10000000;
					IPreviewCPtr->Seek(t);
					DWORD lenth = 0;
					if(it && SUCCEEDED(IPreviewCPtr->GetCurrentImage(NULL,lenth)))
					{
						if (lenth > 0)
						{
							boost::shared_array<BYTE> pBitmapPtr(new BYTE[lenth]);
							if(SUCCEEDED(IPreviewCPtr->GetCurrentImage(pBitmapPtr.get(), lenth)));
							{
								it->set_image(pBitmapPtr, lenth);
								bOK = true;
								emit sig_AsynFresh(task_id);
							}
						}
					}

					IPreviewCPtr->Stop();
				}
			}	
		}
		if (!bOK && it)
		{
			it->set_image(boost::shared_array<BYTE>(),0);//failed
		}
	}
	CoUninitialize();
}

void xListView_ui::remove_selected_items()
{
	bool bNeedNotify = false;
	QList<int>  items = selected_items();
	for (int i = items.size() - 1; i >= 0; i--)
	{
		E_ITME_STATES s = mpTaskManager->get_item_status(items[i]);
		bNeedNotify = s == ITEM_STATES_PAUSE ||  s == ITEM_STATES_TRANSCODING;
		if (bNeedNotify)
			break;
	}
	if (bNeedNotify)
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("The file is being transcoded, are you sure to delete it?"));
		box.SetButtonVisabled(true,true);
		if(QDialog::Rejected == box.exec())
			return;
	}

	for (int i = items.size() - 1; i >= 0; i--)
		mpTaskManager->remove_item(items[i]);
}

void xListView_ui::slot_try(bool)
{
	QModelIndex index = indexAt(mPosMenu);
	QListWidgetItem* it = item(index.row());
	if (it)
	{
		xTaskItemPtr ptr = mpTaskManager->get_item_by_task_id(it->data(0).toInt());
		if (ptr)
		{
			emit sig_try(ptr->get_url());
		}
	}
}