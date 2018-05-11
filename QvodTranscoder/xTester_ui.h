#ifndef XTESTER_H
#define XTESTER_H

#include "xBaseDialog.h"
#include "ui_xTester.h"
#include "xlangchangelisten.h"

class xTaskManagerUI;
class xTester_ui : public xBaseDialog, public xLangChangeListen
{
	Q_OBJECT

public:
	xTester_ui(xTaskManagerUI* pTM, QWidget *parent = 0);
	~xTester_ui();

	virtual void retranslateUi();
public slots:
	void slot_AsynFresh(int);
	void slot_addTask(int);
	void slot_engine_states_changed(E_ENGINE_STATES);

protected:
	void paintEvent(QPaintEvent* event);
private:
	Ui::xTesterClass ui;
	xTaskManagerUI*	mpTaskManager;
	
protected slots:
	virtual void on_bottonClose_clicked();
	virtual void on_close_clicked();
};

#endif // XTESTER_H
