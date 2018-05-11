#include "xListItemDelegate.h"
#include "xListView_ui.h"
#include "xTaskItem.h"
#include "xTaskManagerUI.h"

xListItemDelegate::xListItemDelegate(QObject *parent)
: QItemDelegate(parent)
{
	mListView = (xListView_ui*)parent;
}

xListItemDelegate::~xListItemDelegate()
{

}

extern QSize getFitSize(QSizeF destSize, QSizeF sizePicture);


void xListItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	// currect item
	xTaskItemPtr item = mListView->get_task_manager_ui()->get_item_by_task_id(mListView->item(index.row())->data(0).toInt());
	if (!item)
		return;

	bool bSelected = mListView->selectionModel()->isSelected(index);
	if (bSelected && !item->get_url().isEmpty())
	{
		QRect r = option.rect.adjusted(1,3,-1,0);
		painter->fillRect(r, QColor(128,158,197));
	}

	bool bHoverd = mListView->mouse_hover_item() == index.row();
	// 绘制内容区域
	QRect rect_context = option.rect.adjusted(6,6,-6,-22);
	painter->fillRect(rect_context, QColor(221,221,221));

	{
		// 绘制阴影
		painter->save();
		QRect r =  rect_context.adjusted(1,1,-2,-2);
		QPen pen(Qt::white);
		painter->setPen(pen);
		painter->drawRect(r);
		painter->restore();
	}

	// 绘制缩略图
	{
		QRect rPixmapArea = rect_context.adjusted(2,2,-2,-50);
		Qvod_DetailMediaInfo_Ptr ptr =  mListView->get_task_manager_ui()->get_src_media_info(item->get_task_id());
		if (ptr->nVideoCount < 1)
		{
			painter->drawPixmap(rPixmapArea, QPixmap("images/audio.png"));
		}
		else
		{
			painter->fillRect(rPixmapArea, Qt::black);
			if (item->has_image())
			{
				QImage image = item->get_image();
				QSize size_fit = getFitSize(rPixmapArea.size(),image.size());
				QPoint pos((rPixmapArea.width() - size_fit.width()) / 2, (rPixmapArea.height() - size_fit.height()) / 2);
				pos.setX(rPixmapArea.left()+pos.x());
				pos.setY(rPixmapArea.top()+pos.y());
				painter->drawImage(QRect(pos, size_fit),image);
			}
		}
		
	}


	// 绘制状态

	if (!item->get_url().isEmpty())
	{
		QRect r = rect_context;
		r.setTop(r.bottom() - 20);
		QPoint p = r.topLeft();
		p.setX(p.x() + 3);
		//p.setY(p.y() + 5);
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

		// 字幕
		if (!item->get_title_file_url().isEmpty())
		{
			p.setX(p.x() + 20);
			p.setY(p.y() + 3);
			painter->drawPixmap(p, QPixmap("images/status_title.png"));
		}

		if (item->get_states() == ITEM_STATES_TRANSCODING && mListView->get_task_manager_ui()->is_accel_trans(item->get_task_id()))
		{
			p.setX(p.x() + 20);
			painter->drawPixmap(p, QPixmap("images/status_accel.png"));
		}
	}

	// 文字
	{
		QRect rText = rect_context;
		rText.setTop(rText.bottom() - 35);
		rText.adjust(3,0,-3,0);
		QString url = item->get_display_text();

		painter->save();
		if(bHoverd)
			painter->setPen(QPen(Qt::white));
		else
			painter->setPen(QPen(Qt::black));

		QFontMetrics fm = painter->fontMetrics();
		painter->drawText(rText, Qt::AlignLeft, fm.elidedText(url, Qt::ElideRight, rText.width()));
		painter->restore();
	}

	// 进度
	long progress = item->get_progress();
	if(progress)
	{
		painter->save();
		painter->setRenderHints(QPainter::HighQualityAntialiasing);
		
		QRect rect_bg = rect_context.adjusted(2,0,-2,-41);
		rect_bg.setTop(rect_bg.bottom() - 8);

		// draw background
		QLinearGradient gradient(rect_bg.topLeft(), rect_bg.bottomLeft());
		gradient.setColorAt(0,QColor(182,182,182/*89,89,89*/));
		gradient.setColorAt(0.5,QColor(197,197,197/*99,99,99*/));
		gradient.setColorAt(1,QColor(197,197,197/*89,89,89*/));
		QBrush brush(gradient);
		painter->setBrush(brush);
		painter->setPen(Qt::NoPen);
		painter->drawRect(rect_bg);

		// draw progress
		// /rect_bg.adjust(1,1,-1,-1);
		float pp = progress /10000.0f;
		rect_bg.setWidth(pp * rect_bg.width());

		QLinearGradient gradient2(rect_bg.topLeft(), rect_bg.bottomLeft());
// 		gradient2.setColorAt(0,QColor(242,245,236));
// 		gradient2.setColorAt(0.1,QColor(242,245,236));
// 		gradient2.setColorAt(0.2,QColor(188,237,167));
// 		gradient2.setColorAt(0.9,QColor(125,177,118));
// 		gradient2.setColorAt(1,QColor(152,196,137));
		gradient2.setColorAt(0,QColor(113,189,248));
		gradient2.setColorAt(0.5,QColor(89,175,247));
		gradient2.setColorAt(1,QColor(67,145,244));

		QBrush brush2(gradient2);
		painter->setBrush(brush2);
		
		painter->drawRect(rect_bg);
		painter->restore();
	
		// 剩余时间	
		long tt = item->get_remain_time() / 1000;
		int hh = (tt / 3600) % 24;
		tt %= 3600;
		int mm = tt / 60;
		tt %= 60;
		QTime t(hh, mm, tt);

		QString str = t.toString(QString("HH:mm:ss"));
		QRect rText = option.rect.adjusted(50,0,-10,-5);
		rText.setTop(rText.bottom() - 30);

		if(bHoverd)
			painter->setPen(QPen(Qt::white));
		else
			painter->setPen(QPen(Qt::black));

		painter->drawText(rText, Qt::AlignRight, str);
	}

}

QSize xListItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QSize size = QItemDelegate::sizeHint(option, index);  
	//size.setHeight( size.height() +10 );  
	return size;  
}