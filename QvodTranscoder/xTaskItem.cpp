#include "xTaskItem.h"
#include <QDir>

xTaskItem::xTaskItem(int task_id)
	: mStates(ITEM_STATES_UNKNOWN)
	, mTaskID(task_id)
	, mProgress(0)
	, mStartTime(0)
	, mRemainTime(0)
	, mSProgress(0)
	, mbHasImage(false)
	, mbComplete(false)
{

}

xTaskItem::~xTaskItem()
{

}

void xTaskItem::set_url(QString u)
{
	mURL = QDir::toNativeSeparators(u);
}

void xTaskItem::set_status(E_ITME_STATES s) 
{
	mStates = s;
	if (s != ITEM_STATES_COMPLETE)
		mbComplete = false;/*NOT COMPLETE*/
}

void xTaskItem::set_image(boost::shared_array<BYTE> bitmapPtr, int length)
{
	if (!bitmapPtr)
	{
		mbHasImage = true;
		return;
	}
	BITMAPINFOHEADER* pHeader = (BITMAPINFOHEADER*)bitmapPtr.get();
	mImage = QImage(QSize(pHeader->biWidth, abs(pHeader->biHeight)),QImage::Format_RGB32);
	
	void * src = 0;
	void * dst = 0;
	int strech = pHeader->biWidth*4;
	for (int x = 0; x < abs(pHeader->biHeight); ++x)
	{
		src = (uchar*)pHeader + sizeof(BITMAPINFOHEADER) + x * strech;
		dst = mImage.bits() + x * strech;
		memcpy(dst, src, strech);
	}
	mbHasImage = true;
}

QImage xTaskItem::get_image()
{
	return mImage;
}

QString xTaskItem::get_base_name()
{
	QString name = get_display_text();

	int idx_2 = name.lastIndexOf(QChar('.'));
	if (idx_2 >= 0)
		return name.left(idx_2);
	else
		return name;
}

QString xTaskItem::get_display_text()
{
	int idx = mURL.lastIndexOf(QString("\\"));
	if (idx >= 0)
		return mURL.right(mURL.length() - idx - 1);
	else
		return mURL;
}