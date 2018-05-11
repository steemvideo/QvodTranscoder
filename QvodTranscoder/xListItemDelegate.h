#pragma once

#include <QItemDelegate>

class xListView_ui;
class xListItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	xListItemDelegate(QObject *parent);
	~xListItemDelegate();

protected:
	virtual void	paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	virtual QSize	sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

protected:
	xListView_ui* mListView;
};

