#ifndef XTABLEVIEW_UI_H
#define XTABLEVIEW_UI_H

#include <QTableView>
#include "xIItemOperator.h"
#include "xTaskManagerUI.h"
#include "xlangchangelisten.h"
class xTableModel;

class xTableView_ui : public QTableView , public xIItemOperator,public xLangChangeListen
{
	Q_OBJECT

public:
	xTableView_ui(QWidget *parent);
	~xTableView_ui();

	virtual void		set_task_manager(xTaskManagerUIPtr manager);
	virtual void		fresh(int task_id);
	virtual QList<int>	selected_items();
	virtual void		remove_selected_items();
	virtual void		setActive(bool);
	virtual void		set_selected(int task_id);
	virtual	void		item_status_changed(int ,E_ITME_STATES);
	virtual void		de_selected_all();
	virtual	void		show_item(int task_id);

	xTaskManagerUIPtr	get_task_manager_ui() const { return mpTaskManager;}

	int					mouse_hover_item() { return mMouseHoverRow;}

signals:
	void sig_show_media_info(int taskid);
	void sig_select_changed(int);
	void sig_try(QString url);

protected slots:
	void slot_pause(bool);
	void slot_stop(bool);

	void slot_try(bool);
	void slot_play(bool);
	void slot_play_output(bool);
	void slot_open_source_dir(bool);
	void slot_add_subtitle(bool);
	void slot_remove_subtitle(bool);
	void slot_remove_file(bool);
	void slot_clear_list(bool);
	void slot_media_info(bool);

	void slot_menuAboutToShow();
	void slot_selectChanged(const QItemSelection&,const QItemSelection&);
private:
	void keyPressEvent ( QKeyEvent * event );
	void mouseMoveEvent ( QMouseEvent * event );
	void mousePressEvent (QMouseEvent * event );
	void leaveEvent ( QEvent * event );
	void contextMenuEvent ( QContextMenuEvent * e );

	void dragEnterEvent ( QDragEnterEvent * event );
	void dragMoveEvent ( QDragMoveEvent * event );
	void dropEvent( QDropEvent * event );
	bool event(QEvent *event);
	void init_menu();
	QStringList getDropPath(QList<QUrl>& urls);
	void retranslateUi();
private:
	xTableModel*			mModel;
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
};

#endif // XTABLEVIEW_UI_H
