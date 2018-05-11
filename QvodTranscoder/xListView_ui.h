#ifndef XLISTVIEW_H
#define XLISTVIEW_H

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <QListWidget>
#include "xTaskManagerUI.h"
#include "xlangchangelisten.h"
class xListModel;
#include "xIItemOperator.h"

class xListView_ui : public QListWidget, public xIItemOperator, public xLangChangeListen
{
	Q_OBJECT

public:
	xListView_ui(QWidget *parent);
	~xListView_ui();

	virtual void		set_task_manager(xTaskManagerUIPtr manager);
	virtual void		fresh(int task_id);
	virtual QList<int>  selected_items();
	virtual void		remove_selected_items();
	virtual void		set_selected(int task_id);
	virtual	void		item_status_changed(int ,E_ITME_STATES) {};
	virtual	void		de_selected_all();
	virtual void		setActive(bool);
	virtual	void		show_item(int task_id);

	xTaskManagerUIPtr	get_task_manager_ui() const { return mpTaskManager;}
	int					mouse_hover_item() { return mMouseHoverRow;}

signals:
	void sig_show_media_info(int taskid);
	void sig_select_changed(int);
	void sig_AsynFresh(int);
	void sig_try(QString url);

public slots:
	void	slot_AsynFresh(int task_id);
	void	slot_addTask(int taskId);
	void	slot_removeTask(int taskId);
	void	slot_moveTask(int taskId, int dst_index);
	void	slot_taskCountChanged(int count);
protected slots:

	void slot_pause(bool);
	void slot_stop(bool);

	void slot_play(bool);
	void slot_play_output(bool);
	void slot_open_source_dir(bool);
	void slot_add_subtitle(bool);
	void slot_remove_subtitle(bool);
	void slot_remove_file(bool);
	void slot_clear_list(bool);
	void slot_media_info(bool);
	void slot_try(bool);

	void slot_menuAboutToShow();
	void slot_selectChanged(const QItemSelection&,const QItemSelection&);

protected:
	void keyPressEvent ( QKeyEvent * event );
	void mouseMoveEvent ( QMouseEvent * event );
	void mousePressEvent (QMouseEvent * event );
	void leaveEvent ( QEvent * event );
	void contextMenuEvent ( QContextMenuEvent * e );

	void dragEnterEvent ( QDragEnterEvent * event );
	void dragMoveEvent ( QDragMoveEvent * event );
	void dropEvent( QDropEvent * event );
	//Qt::DropActions supportedDropActions() const { return Qt::ActionMask;}
	bool event(QEvent *event);

	void init_menu();
	QStringList getDropPath(QList<QUrl>& urls);
	void retranslateUi();
	void process_get_image();
private:
	xTaskManagerUIPtr		mpTaskManager;

	int						mMouseHoverRow;

	//Menu
	QMenu*					mMenu;
	QAction*				mActPause;
	QAction*				mActStop;
	QAction*				mActPlay;
	QAction*				mActPlayOutput;
	QAction*				mActOpenSourceDir;
	QAction*				mActAddSubTitle;
	QAction*				mActRemoveSubTitle;
	QAction*				mActRemoveFile;
	QAction*				mActClearList;
	QAction*				mActMediaInfo;
	QAction*				mActTry;

	QPoint					mPosMenu;

	boost::condition					mConGrabImage;
	boost::shared_ptr<boost::thread>	mGrabThread;
	boost::recursive_mutex				mMutex;
	bool								mbTernimal;
};

#endif // XLISTVIEW_H
