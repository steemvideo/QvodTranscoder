#include "xTableItemDelegate.h"
#include "xTableView_ui.h"
#include "xTaskItem.h"
#include "xTableModel.h"
#include "xTableView_ui.h"

xTableItemDelegate::xTableItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{
	mTableView = (xTableView_ui*)parent;
}

xTableItemDelegate::~xTableItemDelegate()
{

}

// QSize getFitSize(QSizeF destSize, QSizeF sizePicture)
// {
// 	if (sizePicture.width() < destSize.width() && sizePicture.height() < destSize.height())
// 		return sizePicture.toSize();
// 
// 	qreal asp_image = sizePicture.width() / sizePicture.height();
// 	qreal asp_dest = destSize.width() / destSize.height();
// 
// 	qreal height = destSize.height();
// 	qreal width = destSize.width();
// 
// 	if (asp_image < asp_dest)
// 	{
// 		height = destSize.height();
// 		width = height * asp_image;
// 	}
// 	else
// 	{
// 		width = destSize.width();
// 		height = width / asp_image;
// 	}
// 	return QSize(width, height);
// }

void xTableItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	// currect item
	xTableModel* model = (xTableModel*)index.model();
	int hover_row = mTableView->mouse_hover_item();
	QStandardItem* it = model->item(index.row(), 0);
	xTaskItemPtr item = mTableView->get_task_manager_ui()->get_item_by_task_id(it->data().toInt());
	bool bHoverd = hover_row == index.row();
	if (index.row() % 2 == 1)
	{
		painter->fillRect(option.rect, QColor(231,232,232));
	}

	bool bSelected = mTableView->selectionModel()->isSelected(index);
	if (bSelected && index.row() < mTableView->get_task_manager_ui()->item_count())
	{
		painter->fillRect(option.rect, QColor(129,158,198));
	}
	else if(bHoverd)
	{
		painter->fillRect(option.rect, QColor(223,228,243));
	}

	if (item)
	{
		// status
		if(index.column() == 0)
		{
			if (!item->get_url().isEmpty())
			{
				QPoint p = option.rect.topLeft();
				p.setX(p.x() + 12);
				p.setY(p.y() + 5);
				if (item->get_states() == ITEM_STATES_WAIT)
					painter->drawPixmap(p,QPixmap("images/status_wait.png"));
				else if (item->get_states() == ITEM_STATES_TRANSCODING)
					painter->drawPixmap(p,QPixmap("images/status_transcoding.png"));
				else if (item->get_states() == ITEM_STATES_PAUSE)
					painter->drawPixmap(p,QPixmap("images/status_pause.png"));
				else if (item->get_states() == ITEM_STATES_STOP)
					painter->drawPixmap(p,QPixmap("images/status_stop.png"));
				else if (item->get_states() == ITEM_STATES_COMPLETE)
					painter->drawPixmap(p,QPixmap("images/status_complete.png"));
				else if(item->get_states() == ITEM_STATES_ERROR)
					painter->drawPixmap(p,QPixmap("images/status_error.png"));
			}
		}
		// url
		else if(index.column() == 1)
		{
			QString url = item->get_display_text();
			QRect r = option.rect.adjusted(10,0,0,0);

			if(bHoverd)
				painter->setPen(QPen(Qt::white));
			else
				painter->setPen(QPen(Qt::black));

			
			QFontMetrics fm = painter->fontMetrics();
			painter->drawText(r, Qt::AlignLeft |  Qt::AlignVCenter, fm.elidedText(url, Qt::ElideRight, r.width()));
			
			if (!item->get_title_file_url().isEmpty())
			{
				QPoint p = option.rect.topRight();
				p.setX(p.x() - 20);
				p.setY(p.y() + 10);
				painter->drawPixmap(p, QPixmap("images/status_title.png"));
			}

			if (item->get_states() == ITEM_STATES_TRANSCODING && mTableView->get_task_manager_ui()->is_accel_trans(item->get_task_id()))
			{
				QPoint p = option.rect.topRight();
				p.setX(p.x() - 40);
				p.setY(p.y() + 10);
				painter->drawPixmap(p, QPixmap("images/status_accel.png"));
			}
		}
		// progress
		else if(index.column() == 2)
		{
			long progress = item->get_progress();
			if(progress)
			{
				painter->save();

				painter->setRenderHints(QPainter::Antialiasing);
				
				QRect rect_bg = option.rect;
				rect_bg.adjust(10,10,-50,-10);
				rect_bg.setHeight(9);

				painter->setPen(Qt::NoPen);
				//painter->setPen(QPen(QColor(/*89,89,89*/197,197,197)));

				// draw background
				QLinearGradient gradient(rect_bg.topLeft(), rect_bg.bottomLeft());
				gradient.setColorAt(0,QColor(182,182,182/*89,89,89*/));
				gradient.setColorAt(0.5,QColor(197,197,197/*99,99,99*/));
				gradient.setColorAt(1,QColor(197,197,197/*89,89,89*/));
				QBrush brush(gradient);
				painter->setBrush(brush);
				
				painter->drawRoundedRect(rect_bg, 3,3);

				QRect rProgress = option.rect;
				rProgress.adjust(10,10,-50,-10);
				rProgress.setHeight(9);

				//rect_bg.setHeight(7);
				// draw progress
				float pp = progress /10000.0f;
				rProgress.setWidth(pp * rProgress.width());
			
				QLinearGradient gradient2(rProgress.topLeft(), rProgress.bottomLeft());
				/*gradient2.setColorAt(0,QColor(242,245,236));
				gradient2.setColorAt(0.1,QColor(242,245,236));
				gradient2.setColorAt(0.2,QColor(188,237,167));
				gradient2.setColorAt(0.9,QColor(125,177,118));
				gradient2.setColorAt(1,QColor(152,196,137));
				*/

				gradient2.setColorAt(0,QColor(113,189,248));
				gradient2.setColorAt(0.5,QColor(89,175,247));
				gradient2.setColorAt(1,QColor(67,145,244));

				QBrush brush2(gradient2);
				painter->setBrush(brush2);

				//painter->setPen(Qt::NoPen);
				painter->drawRoundedRect(rProgress, 3,3);

				QString strPP = QString::number(pp*100,'f',1) + QString("%");
				if(bHoverd)
					painter->setPen(QPen(Qt::white));
				else
					painter->setPen(QPen(Qt::black));
				painter->drawText(option.rect.bottomRight() - QPoint(40,8),strPP);
				painter->restore();
			}

		}
		// time left
		else if(index.column() == 3)
		{
			long progress = item->get_progress();
			if(progress)
			{
				long tt = item->get_remain_time() / 1000;
				int hh = (tt / 3600) % 24;
				tt %= 3600;
				int mm = tt / 60;
				tt %= 60;
				QTime t(hh, mm, tt);

				QString str = t.toString(QString("HH:mm:ss"));

				if(bHoverd)
					painter->setPen(QPen(Qt::white));

				painter->drawText(option.rect, Qt::AlignHCenter |  Qt::AlignVCenter, str);
			}
		}
	}
}

QSize xTableItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);  
	//size.setHeight( size.height() +10 );  
	return size;  
}