#include "ImageCanvas.h"
#include <QPainter>
#include <QRectF>

class ImageCanvas::Internal
{
public:
	ImageCanvas* Owner = 0;

	Internal(ImageCanvas* owner) : Owner(owner) {}
	~Internal() {}

public:
	bool m_antialising = true;
	QImage m_image;
	QSize m_prevSize = QSize(0, 0);
};


ImageCanvas::ImageCanvas(QWidget* parent)
	: QWidget(parent)
	, pImpl(new Internal(this))
{

}

QSize ImageCanvas::minimumSizeHint() const
{
	return QSize(100, 100);
}

QSize ImageCanvas::sizeHint() const
{
	return QSize(400, 200);
}

void ImageCanvas::setImage(QImage const& image)
{
	pImpl->m_image = image.copy();
	update();
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
	QImage image = pImpl->m_image.copy();
	
	const QSize curSize(image.width(), image.height());
	if (curSize != pImpl->m_prevSize)
	{
		this->resize(curSize);
		pImpl->m_prevSize = curSize;
	}
	
	QPainter painter(this);
	//painter.setBrush(QBrush(image));

	if (!image.isNull())
	{
		painter.drawImage(QRect(QPoint(0, 0), curSize), image);
	}
	
}
