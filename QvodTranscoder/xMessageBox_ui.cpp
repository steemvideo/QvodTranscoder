#include "xMessageBox_ui.h"

xMessageBox::xMessageBox(QWidget *parent)
	: QDialog(parent,Qt::FramelessWindowHint)
	, mbMoveable(false)
	,mSecondToClose(0)
	,mSecondCast(0)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_TranslucentBackground);
	
	DisableMiniButton();

	connect(&mTimer, SIGNAL(timeout()), this, SLOT(slot_timeout()));
	connect(ui.widget, SIGNAL(sig_close()), this, SLOT(on_btnCancel_clicked()));
}

xMessageBox::~xMessageBox()
{

}

void xMessageBox::retranslateUi()
{
	ui.retranslateUi(this);

}

void xMessageBox::DisableMiniButton()
{
	ui.widget->disable_mini_button();
}

void xMessageBox::slot_timeout()
{
	mSecondCast++;
	if (mSecondCast == mSecondToClose)
	{
		accept();
	}
	else
	{
		QString text = mAutoText;
		text.replace("%d",QString::number(mSecondToClose - mSecondCast));
		ui.lblText->setText(text);
	}
}

void xMessageBox::SetButtonText(int index, QString str)
{
	if (!index)
		ui.btnOK->setText(str);
	else
		ui.btnCancel->setText(str);
}

void xMessageBox::on_btnOK_clicked()
{
	accept();
}

void xMessageBox::SetTitle(QString t)
{
	ui.widget->set_title(t);
}

void xMessageBox::SetText(QString str, QString auto_fill, int auto_close_second)
{
	if (!auto_fill.isEmpty())
	{
		mAutoText = str;
		str.replace(auto_fill, QString::number(auto_close_second));

		mSecondToClose = auto_close_second;
		mTimer.start(1000);
	}
	ui.lblText->setText(str);
}

void xMessageBox::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::HighQualityAntialiasing);

	QPen pen(QColor(172,172,172));
	pen.setWidth(1);
	painter.setPen(pen);

	QLinearGradient gradient(QPoint(0,0),QPoint(0,height()));
	gradient.setColorAt(0,QColor(236,236,236));
	gradient.setColorAt(0.8,QColor(225,225,225));
	gradient.setColorAt(1,QColor(215,215,215));
	QBrush brush(gradient);
	//QBrush brush(QColor(236,236,236));
	painter.setBrush(brush);


	QRect rect_bg = rect();
	rect_bg.adjust(0,0,-1,-1);
	painter.drawRoundedRect(rect_bg, 5,5);


	QPen pen2(QColor(255,255,255));
	pen2.setWidth(1);

	painter.setPen(pen2);
	rect_bg.adjust(1,1,-1,-1);
	painter.drawRoundedRect(rect_bg, 5,5);

}

void xMessageBox::mousePressEvent(QMouseEvent *event)  
{  
	if(event->button() == Qt::LeftButton)  
	{  
		mDragPosition = event->globalPos() - frameGeometry().topLeft();    
		mbMoveable = true;  

		event->accept();  
	}  
}  

void xMessageBox::mouseMoveEvent(QMouseEvent *event)  
{  
	if(event->buttons() & Qt::LeftButton && mbMoveable)  
	{  
		move(event->globalPos() - mDragPosition);  
		event->accept();  
	}  
}  

void xMessageBox::mouseReleaseEvent(QMouseEvent* event)  
{  
	if(mbMoveable)  
	{  
		mbMoveable = false;  
	}  
}  


void xMessageBox::on_btnCancel_clicked()
{
	reject();
}

void xMessageBox::SetButtonVisabled(bool bOK,bool bCancel)
{
	ui.btnOK->setHidden(!bOK);
	ui.btnCancel->setHidden(!bCancel);
}