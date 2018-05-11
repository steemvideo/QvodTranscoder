#pragma once

class xTaskManagerUI;
class xIItemOperator
{
public:
	virtual	void		set_task_manager(boost::shared_ptr<xTaskManagerUI>) = 0;
	virtual	void		fresh(int task_id) = 0;
	virtual QList<int>	selected_items() = 0;
	virtual void		remove_selected_items() = 0;
	virtual void		set_selected(int task_id) = 0;
	virtual void		de_selected_all() = 0;
	virtual	void		item_status_changed(int, E_ITME_STATES) = 0;
	virtual	void		show_item(int task_id) = 0;
	virtual void		setActive(bool) = 0;
};

