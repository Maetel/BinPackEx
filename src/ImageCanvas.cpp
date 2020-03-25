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

	ImageCanvas::CanvasObjectType m_showWhat = ImageCanvas::ALL;
	QBrush m_KarlsunBrush;
	std::vector<Karlsun> m_karlsuns;
	QImage m_image;
	QSize m_prevSize = QSize(0, 0);

	bool showImage() { return m_showWhat & ImageCanvas::ImageObj; }
	bool showKarlsun() { return m_showWhat & ImageCanvas::KarlsunObj; }
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

void ImageCanvas::resetCanvas()
{
	//pImpl->m_KarlsunBrush = QBrush //keep this
	pImpl->m_karlsuns.clear();
	pImpl->m_image = QImage();
	update();
}

void ImageCanvas::showObejct(CanvasObjectType objType)
{
	int curState = pImpl->m_showWhat;
	int received = objType;
	curState |= received;
	pImpl->m_showWhat = (CanvasObjectType)curState;
	update();
}
void ImageCanvas::hideObejct(CanvasObjectType objType)
{
	int curState = pImpl->m_showWhat;
	int received = ~objType;
	curState &= received;
	pImpl->m_showWhat = (CanvasObjectType)curState;
	update();
}

void ImageCanvas::setKarlsunBrush(QBrush brush)
{
	pImpl->m_KarlsunBrush = brush;
	update();
}

void ImageCanvas::setKarlsun(std::vector<Karlsun> const& rects)
{
	pImpl->m_karlsuns = rects;
	update();
}

void ImageCanvas::setImage(QImage const& image)
{
	pImpl->m_image = image.copy();
	update();
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
	//QImage image = pImpl->m_image.copy();
	QImage const& image = pImpl->m_image;
	auto const& rects = pImpl->m_karlsuns;
	auto const& rectBrush = pImpl->m_KarlsunBrush;

	const QSize curSize(image.width(), image.height());
	if (curSize != pImpl->m_prevSize)
	{
		this->resize(curSize);
		pImpl->m_prevSize = curSize;
	}
	
	QPainter painter(this);
	//painter.setBrush(QBrush(image));

	if (!image.isNull() && pImpl->showImage())
	{
		painter.drawImage(QRect(QPoint(0, 0), curSize), image);
	}

	if (!rects.empty() && pImpl->showKarlsun())
	{
		for (auto const& karlsun : rects)
		{
			QRect rect = karlsun.rect;
			QBrush rectBrush;
			rectBrush.setColor(karlsun.color);
			painter.setBrush(rectBrush);
			painter.drawRoundedRect(rect, karlsun.roundPixel, karlsun.roundPixel);
		}
	}

	//painter.drawRoundedRect
	
}
