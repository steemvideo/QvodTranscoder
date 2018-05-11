#include "xBaseDialog.h"

xBaseDialog::xBaseDialog(QWidget *parent)
	: QDialog(parent,Qt::FramelessWindowHint)
	, mbMoveable(false)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setupUi();
}

xBaseDialog::~xBaseDialog()
{

}

void xBaseDialog::setupUi()
{
	setObjectName(QString::fromUtf8("xBaseDialogClass"));

	mTitleBar = new xTitleBar_ui(this);
	mTitleBar->setObjectName(QString::fromUtf8("xTitleBarClass"));
	mTitleBar->setMinimumSize(QSize(100, 25));
	mTitleBar->setMaximumSize(QSize(16777215, 25));
	mTitleBar->move(5,2);

	QObject::connect(mTitleBar,SIGNAL(sig_close()), this,SLOT(on_close_clicked()));
} 

void xBaseDialog::resizeEvent ( QResizeEvent * event)
{
	mTitleBar->resize(event->size().width()-10, 25);
	QDialog::resizeEvent(event);
}

void xBaseDialog::setTitle(QString t)
{
	mTitleBar->set_title(t);
}

void xBaseDialog::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::HighQualityAntialiasing);

	QPen pen(QColor(172,172,172));
	pen.setWidth(1);
	painter.setPen(pen);

	QLinearGradient gradient(QPoint(0,0),QPoint(0,height()));
	gradient.setColorAt(0,QColor(251,251,251));
	gradient.setColorAt(0.1,QColor(232,232,232));
	gradient.setColorAt(0.2,QColor(233,233,233));
	gradient.setColorAt(0.8,QColor(225,225,225));
	gradient.setColorAt(1,QColor(215,215,215));
	QBrush brush(gradient);
	painter.setBrush(brush);


	QRect rect_bg = rect();
	rect_bg.adjust(0,0,-1,-1);
	painter.drawRoundedRect(rect_bg, 5,5);


	QPen pen2(QColor(255,255,255));
	pen2.setWidth(1);

	painter.setPen(pen2);
	rect_bg.adjust(1,1,-1,-1);
	painter.drawRoundedRect(rect_bg, 5,5);

	

	QPen p(QColor(193,193,193));
	p.setWidth(1);
	painter.setPen(p);
	painter.drawLine(QPoint(0,30), QPoint(width(),30));
}

void xBaseDialog::mousePressEvent(QMouseEvent *event)  
{  
	if(event->button() == Qt::LeftButton)  
	{  
		mDragPosition = event->globalPos() - frameGeometry().topLeft();    
		mbMoveable = true;  

		event->accept();  
	}  
}  

void xBaseDialog::mouseMoveEvent(QMouseEvent *event)  
{  
	if(event->buttons() & Qt::LeftButton && mbMoveable)  
	{  
		move(event->globalPos() - mDragPosition);  
		event->accept();  
	}  
}  

void xBaseDialog::mouseReleaseEvent(QMouseEvent* event)  
{  
	if(mbMoveable)  
	{  
		mbMoveable = false;  
	}  
}  

void xBaseDialog::on_close_clicked()
{
	accept();
}