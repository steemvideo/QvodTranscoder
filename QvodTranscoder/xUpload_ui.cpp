#include "xUpload_ui.h"
#include <QItemDelegate>
#include "xFunction.h"
#include "xApp.h"
#include "xMessageBox_ui.h"

/************************************************************************/
/* xUploadItemDelegate类                                                */
/************************************************************************/
class xUploadItemDelegate : public QItemDelegate
{
public:
	xUploadItemDelegate(QTableView* view,QObject* parent = NULL);

	QTableView*	mpTableView;
	QStandardItemModel* mpModel;
public:
	virtual void paint (QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};


xUploadItemDelegate::xUploadItemDelegate(QTableView* view,QObject* parent)
: QItemDelegate(parent),mpTableView(view)
{
	mpModel = (QStandardItemModel*)view->model();
}

void xUploadItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QStandardItem* item_url = mpModel->item(index.row(), 1);
	QString url = item_url ? item_url->data().toString() : "";

	if (index.row() % 2 == 1)
	{
		painter->fillRect(option.rect, QColor(231,232,232));
	}

	bool bSelected = mpTableView->selectionModel()->isSelected(index);
	if (bSelected && index.row() < mpModel->rowCount())
	{
		painter->fillRect(option.rect, QColor(129,158,198));
	}

	if (index.column() == 0)
	{
		if(!url.isEmpty())
		{
			QStandardItem* item_select = mpModel->item(index.row(), 0);
			QPixmap pxp =  QPixmap(item_select->data().toBool() ? "images\\checkbox_checked.png" : "images\\checkbox_unchecked.png");
			QPoint p = option.rect.topLeft();
			p.setX(p.x() + 12);
			p.setY(p.y() + 6);

			painter->drawPixmap(p,pxp);
		}
	}
	else if(index.column() == 1)
	{
		if (url.size())
		{
			QRect r = option.rect.adjusted(10,0,0,0);
			painter->setPen(QPen(Qt::black));

			int pos = url.lastIndexOf("\\");
			if (pos != -1)
			{
				QFontMetrics fm = painter->fontMetrics();

				QString baseName = url.mid(pos+1);
				painter->drawText(r, Qt::AlignLeft |  Qt::AlignVCenter, fm.elidedText(baseName, Qt::ElideRight, r.width()));
			}
			
		}
	}
	else if(index.column() == 2)
	{
		if (url.size())
		{
			QString strSize = mpModel->item(index.row(),2)->data().toString();
			QRect r = option.rect.adjusted(5,0,0,0);
			painter->setPen(QPen(Qt::black));
			QFontMetrics fm = painter->fontMetrics();
			painter->drawText(r, Qt::AlignLeft |  Qt::AlignVCenter, fm.elidedText(strSize, Qt::ElideRight, r.width()));
		}
	}
	else
	{
		float progress = mpModel->item(index.row(), 3)->data().toFloat();
		if(progress > 0.00f)
		{
			painter->save();
			painter->setRenderHints(QPainter::Antialiasing);

			QRect rect_bg = option.rect;
			rect_bg.adjust(10,10,-50,-10);
			rect_bg.setHeight(9);

			painter->setPen(QPen(QColor(89,89,89)));

			// draw background
			QLinearGradient gradient(rect_bg.topLeft(), rect_bg.bottomLeft());
			gradient.setColorAt(0,QColor(89,89,89));
			gradient.setColorAt(0.5,QColor(99,99,99));
			gradient.setColorAt(1,QColor(89,89,89));
			QBrush brush(gradient);
			painter->setBrush(brush);

			painter->drawRoundedRect(rect_bg, 3,3);

			QRect rProgress = option.rect;
			rProgress.adjust(11,11,-51,-11);
			rProgress.setHeight(7);

			// draw progress
			rProgress.setWidth(progress * rProgress.width());

			QLinearGradient gradient2(rProgress.topLeft(), rProgress.bottomLeft());
			gradient2.setColorAt(0,QColor(242,245,236));
			gradient2.setColorAt(0.1,QColor(242,245,236));
			gradient2.setColorAt(0.2,QColor(188,237,167));
			gradient2.setColorAt(0.9,QColor(125,177,118));
			gradient2.setColorAt(1,QColor(152,196,137));
			QBrush brush2(gradient2);
			painter->setBrush(brush2);

			painter->setPen(Qt::NoPen);
			painter->drawRoundedRect(rProgress, 3,3);

			QString strPP = QString::number(progress*100,'f',1) + QString("%");
			painter->setPen(QPen(Qt::black));
			painter->drawText(option.rect.bottomRight() - QPoint(40,8),strPP);
			painter->restore();
		}
	}

	
}


/************************************************************************/
/* xUpload_ui                                                           */
/************************************************************************/
xUpload_ui::xUpload_ui(QWidget *parent)
	: xBaseDialog(parent)
	, mbReciveUserCMD(false)
	, mbCover(false)
{
	ui.setupUi(this);
	mTitleBar->set_title(tr("Copy"));
	mTitleBar->disable_mini_button();

	mpModel = new QStandardItemModel(this);
	mpModel->setColumnCount(4);
	mpModel->setHeaderData(0,Qt::Horizontal,tr("Select"));
	mpModel->setHeaderData(1,Qt::Horizontal,tr("File name"));
	mpModel->setHeaderData(2,Qt::Horizontal,tr("Size"));
	mpModel->setHeaderData(3,Qt::Horizontal,tr("Progress"));

	ui.tableView->setModel(mpModel);
	ui.tableView->setItemDelegate(new xUploadItemDelegate(ui.tableView, this));	

	ui.tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);


	ui.tableView->setColumnWidth(0,45);
	ui.tableView->setColumnWidth(1,195);
	ui.tableView->setColumnWidth(2,50);
	ui.tableView->setColumnWidth(3,113);

	ui.tableView->horizontalHeader()->setResizeMode(0,QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setResizeMode(3,QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setClickable(false);
	ui.tableView->horizontalHeader()->setMovable(false);
	ui.tableView->horizontalHeader()->setMinimumSectionSize(50);
	ui.tableView->horizontalHeader()->setStretchLastSection(true);

	ui.tableView->viewport()->installEventFilter(this);
	connect(this,SIGNAL(sig_AsynFreshRow(int)),this, SLOT(slot_AsynFreshRow(int)), Qt::QueuedConnection);
	connect(this,SIGNAL(sig_AsynComplete()),this, SLOT(slot_AsynComplete()), Qt::QueuedConnection);
	connect(this,SIGNAL(sig_requestUserCMD()),this, SLOT(slot_requestUserCMD()), Qt::QueuedConnection);
	
	connect(QvodApplication::instance(),SIGNAL(sig_DeviceChanged(bool,QString)),this,SLOT(slot_DeviceChanged(bool,QString)));
	
	// DEVICE DETECT
	int_logical_driver();


	//Add EMPTY ROW
	for (int i = 0; i < 7; i++)
	{
		add_row("");
	}

	//ADD URLs
	xTaskManagerUIPtr taskManager = QvodApplication::instance()->mMainWnd->get_taskManager();
	if (taskManager)
	{
		int	count = taskManager->item_count();
		for(int i = 0; i < count; i++)
		{
			xTaskItemPtr it = taskManager->get_item_by_index(i);
			if (it->get_states() == ITEM_STATES_COMPLETE)
				add_row(it->get_dest_url());
		}
	}

	if (mpModel->rowCount() == 0)
	{
		ui.btnCopy->setEnabled(false);
	}
}

void xUpload_ui::int_logical_driver()
{
	TCHAR szDriverBuffer[512];
	DWORD dwLength = GetLogicalDriveStrings(512, szDriverBuffer);
	TCHAR *lpDriverBuffer = szDriverBuffer;
	while (*lpDriverBuffer != NULL)
	{
		QString str = QString::fromWCharArray(lpDriverBuffer);
		lpDriverBuffer = lpDriverBuffer + str.size() + 1;

		//is removable device?
		if(DRIVE_REMOVABLE == GetDriveType(str.toStdWString().c_str()))
			slot_DeviceChanged(true,str);
	}

	if (ui.comboBox->count() == 0)
	{
		ui.comboBox->addItem(tr("Please insert your external device!"));
		ui.btnCopy->setEnabled(false); 
	}
	else
	{
		ui.btnCopy->setEnabled(true);
	}

}

void xUpload_ui::slot_DeviceChanged(bool bAdd,QString strDrive)
{
	//正在複製時被移除？
	if (!bAdd)
	{
		if (ui.comboBox->currentText().contains(strDrive,Qt::CaseInsensitive))
		{
			mbCanceled = true;
		}
	}

	if (!ui.btnCopy->isEnabled())
	{
		ui.comboBox->clear();
	}

	bool bExsit = false;
	for (int j = 0; j < ui.comboBox->count(); j++)
	{
		QString text = ui.comboBox->itemText(j);
		if (text.contains(strDrive, Qt::CaseInsensitive))
		{
			bExsit = true;
			if (!bAdd)
			{
				ui.comboBox->removeItem(j);
				break;
			}
		}
	}
	if (bAdd && !bExsit)
	{
		ui.comboBox->addItem(strDrive);
	}

	if (ui.comboBox->count() == 0)
	{
		ui.comboBox->addItem(tr("Please insert your external device!"));
		ui.btnCopy->setEnabled(false); 
	}
	else
	{
		ui.btnCopy->setEnabled(true);
	}

	
}

void xUpload_ui::retranslateUi() 
{
	ui.retranslateUi(this);

	mpModel->setHeaderData(0,Qt::Horizontal,tr("Select"));
	mpModel->setHeaderData(1,Qt::Horizontal,tr("File name"));
	mpModel->setHeaderData(2,Qt::Horizontal,tr("Size"));
	mpModel->setHeaderData(3,Qt::Horizontal,tr("Progress"));
}

xUpload_ui::~xUpload_ui()
{
	
}

void xUpload_ui::add_row(QString url)
{
	int row = 0;
	// 找到一个空行放置
	if (!url.isEmpty())
	{
		for(row = 0; row < mpModel->rowCount(); row++)
		{
			QStandardItem* item_url = mpModel->item(row,1);
			if(item_url->data().toString().isEmpty())
			{
				mpModel->item(row,0)->setData(true);
				
				QFile file(url);
				float size = file.size();
				QString tt= QString::number(size/1000000,'f',2);
				mpModel->item(row,2)->setData( tt+ QString("MB"));

				item_url->setData(url);
				return;
			}	
		}
	}

	if(url.isEmpty() || row == mpModel->rowCount())
	{
		//Append
		QList<QStandardItem *> items_list;

		//status
		QStandardItem* item_select = new QStandardItem();
		item_select->setSelectable(true);
		item_select->setEditable(false);

		//url
		QStandardItem* item_url = new QStandardItem();
		item_url->setSelectable(true);
		item_url->setEditable(false);
		item_url->setData(url);
		item_url->setToolTip(url);

		//progress
		QStandardItem* item_size = new QStandardItem();
		item_size->setSelectable(true);
		item_size->setEditable(false);
		QFile file(url);
		item_size->setData(QString::number(file.size()/(1024*1024),'f',2) + QString("MB"));

		//time leave
		QStandardItem* item_progress = new QStandardItem();
		item_progress->setSelectable(true);
		item_progress->setEditable(false);
		

		items_list << item_select << item_url << item_size << item_progress;

		mpModel->appendRow(items_list);
	}
}

void xUpload_ui::on_btnQuit_clicked()
{
	if (mThread && !mbQuitThread)
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("Copy file now, are you sure to quit?!"));
		box.SetButtonVisabled(true,false);
		if(QDialog::Rejected == box.exec())
			return;
	}
	if (mThread)
	{
		mbCanceled = true;

		mThread->join();
		mThread.reset();
	}
	reject();
}

void xUpload_ui::on_btnSelectDirectory_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder"),
		ui.comboBox->currentText(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (!dir.isEmpty())
	{
		QString d = QDir::toNativeSeparators(dir);
		ui.comboBox->lineEdit()->setText(d);

		ui.btnCopy->setEnabled(true);
	}
}

void xUpload_ui::on_btnOpenDirectory_clicked()
{
	if (ui.comboBox->currentText().isEmpty())
	{
		return;
	}
	g_open_dir(ui.comboBox->currentText());
}

bool xUpload_ui::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) 
	{
		QMouseEvent *keyEvent = static_cast<QMouseEvent *>(event);
		QModelIndex index = ui.tableView->indexAt(keyEvent->pos());
		QStandardItem* item_select = mpModel->item(index.row(),1);
		if (item_select->data().toString().isEmpty())
		{
			ui.tableView->selectionModel()->clear();
			return true;
		}

		if (index.column() == 0)
		{
			QStandardItem* item_select = mpModel->item(index.row(),index.column());
			item_select->setData(!item_select->data().toBool());
			ui.tableView->update(index);
		}
	}		
	// standard event processing
	return QObject::eventFilter(obj, event);
}

void xUpload_ui::slot_AsynFreshRow(int row)
{
	ui.tableView->update(mpModel->index(row,3));
}

void xUpload_ui::on_btnCopy_clicked()
{
	if (mpModel->rowCount() == 0)
	{
		return;
	}

	if (ui.comboBox->currentText().isEmpty())
	{
		xMessageBox box(this);
		box.SetTitle(tr("Tips"));
		box.SetText(tr("Please select valid output folder"));
		box.SetButtonVisabled(true,false);
		box.exec();
		return;
	}

	QString str = ui.comboBox->currentText();
	QDir d(str);
	if (!d.exists())
	{
		d.mkpath(str);
	}
 
 	if (str == mOutputDir)
 	{
 		xMessageBox box(this);
 		box.SetTitle(tr("Tips"));
 		box.SetText(tr("Output directory is same!"));
 		box.SetButtonVisabled(true,false);
 		box.exec();
 		return;
 	}

	if (str[str.length()-1] != '\\')
	{
		str += "\\";
	}

	
	if (mThread)
	{
		mbCanceled = true;
		mThread->join();
	}
	mDestPath = str;
	mbCanceled = false;
	mThread.reset(new boost::thread( boost::bind(&xUpload_ui::process_copy, this)));


	ui.comboBox->setEnabled(false);
}

void xUpload_ui::process_copy()
{
	mbQuitThread = false;

	for(int i = 0; i < mpModel->rowCount(); i++)
	{
		if (!mpModel->item(i,0)->data().toBool())
			continue;

		QString strSrc = mpModel->item(i, 1)->data().toString();
		if (!strSrc.isEmpty())
		{
			char buffer[4096] = { 0 };
			
			QFile fileSrc(strSrc);
			fileSrc.open(QIODevice::ReadOnly);
			QDataStream in(&fileSrc);    // read the data serialized from the file
			
			int sizeCount = fileSrc.size();
			int pos = strSrc.lastIndexOf("\\");
			QString baseName = strSrc.mid(pos+1);

			QString destUrls = mDestPath + baseName;
			if (QFile::exists(destUrls))
			{
				if (destUrls == strSrc)
				{
					// 直接100%
					mpModel->item(i,3)->setData(1.0f);
					emit sig_AsynFreshRow(i);
					continue;
				}

				// 請求用戶決定是否覆蓋
				mbReciveUserCMD = false;
				emit sig_requestUserCMD();
				while(!mbReciveUserCMD)
				{
					Sleep(500);
				}

				if (!mbCover)
				{
					continue;
				}
			}

			QFile fileDet(destUrls);
			if(fileDet.open(QIODevice::WriteOnly))
			{
				QDataStream out(&fileDet);   // we will serialize the data into the file

				int prgress = 0;
				int size = 0;
				while((size = in.readRawData(buffer,4096)) > 0)
				{
					if(mbCanceled)
						break;

					if(size == out.writeRawData(buffer,size))
					{
						// 进度更新
						prgress += size;
						mpModel->item(i,3)->setData(prgress / (float)sizeCount);
						emit sig_AsynFreshRow(i);
					}
				}
			}
			
			fileSrc.close();
			fileDet.close();

			if(mbCanceled)
			{
				// remove file
				QFile::remove(mDestPath + strSrc);
				break;
			}
		}
	}

	mbQuitThread = true;
	emit sig_AsynComplete();
}

void xUpload_ui::slot_AsynComplete()
{
	ui.btnCopy->setEnabled(true);
	ui.btnSelectDirectory->setEnabled(true);
	ui.comboBox->setEnabled(true);
}

void xUpload_ui::slot_requestUserCMD()
{
	xMessageBox box(this);
	box.SetTitle(tr("Tips"));
	box.SetText(tr("File exsits!replace it now?"));
	box.SetButtonVisabled(true,true);
	box.SetButtonText(0,tr("Replace"));
	mbCover = QDialog::Accepted == box.exec();
	mbReciveUserCMD = true;
}