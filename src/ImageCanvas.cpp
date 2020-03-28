#include "ImageCanvas.h"
#include <QPainter>
#include <QRectF>

class ImageCanvas::Internal
{
public:
	ImageCanvas* Owner = 0;

	Internal(ImageCanvas* owner)
		: Owner(owner)
		, minWid(Owner->minimumSizeHint().width())
		, minHi(Owner->minimumSizeHint().height())
	{}
	~Internal() {}

public:
	bool m_antialising = true;

	ImageCanvas::CanvasObjectType m_showWhat = ImageCanvas::ALL;
	QBrush m_KarlsunBrush;
	std::vector<Karlsun> m_karlsuns;
	QImage m_image;
	QSize m_prevSize = QSize(0, 0);

	bool showImage() const { return m_showWhat & ImageCanvas::ImageObj; }
	bool showKarlsun() const { return m_showWhat & ImageCanvas::KarlsunObj; }
	
	const int minWid, minHi;

	QSize chooseCanvasSize()
	{
		QSize retval = m_prevSize;

		//update if different
		if (const auto curSize = m_image.size(); m_prevSize != curSize)
			retval = curSize;

		//check min
		{
			if (retval.width() < minWid)
				retval.setWidth(minWid);
			if (retval.height() < minHi)
				retval.setHeight(minHi);
		}

		//store last size
		m_prevSize = retval;
		return retval;
	}
};


ImageCanvas::ImageCanvas(QWidget* parent)
	: QWidget(parent)
	, pImpl(new Internal(this))
{
}

QSize ImageCanvas::minimumSizeHint() const
{
	return QSize(640, 480);
}

QSize ImageCanvas::sizeHint() const
{
	return QSize(640, 480);
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
	const QSize szDebug = pImpl->m_image.size();
	update();
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
	//QImage image = pImpl->m_image.copy();
	QImage const& image = pImpl->m_image;
	auto const& rects = pImpl->m_karlsuns;
	auto const& rectBrush = pImpl->m_KarlsunBrush;

	const QSize curSize = pImpl->chooseCanvasSize();
	this->resize(curSize);

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
			rectBrush.setColor(karlsun.style.color);
			painter.setBrush(rectBrush);
			painter.drawRoundedRect(rect, karlsun.style.roundPixel, karlsun.style.roundPixel);
		}
	}

	//painter.drawRoundedRect
	
}
