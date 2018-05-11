#ifndef XTABLEVIEW_DELEGATE_H
#define XTABLEVIEW_DELEGATE_H

#include <QItemDelegate>
#include <QTableView>

class xTableView_ui;
class xTableItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	xTableItemDelegate(QObject *parent);
	~xTableItemDelegate();
	
protected:
	virtual void	paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	virtual QSize	sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

protected:
	xTableView_ui* mTableView;

};

#endif // XTABLEVIEW_DELEGATE_H
