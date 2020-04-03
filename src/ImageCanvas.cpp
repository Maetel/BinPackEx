#include "ImageCanvas.h"
#include <QPainter>
#include <QRectF>
#include <QTextItem>
#include "BinImage.h"

class ImageCanvas::Internal
{
public:
	ImageCanvas* Owner = 0;

	Internal(ImageCanvas* owner)
		: Owner(owner)
	{}
	~Internal() {}

public:
	bool m_antialising = true;

	ImageCanvas::CanvasObjectType m_showWhat = ImageCanvas::ALL;
	QBrush m_KarlsunBrush;
	std::vector<BinImagePtr> m_binImages;
	std::vector<QImage> m_binQImages;
	std::vector<Karlsun> m_karlsuns;

	std::atomic_int m_canvasPadding = 20;
	QPoint canvasPadding() const { return QPoint(m_canvasPadding, m_canvasPadding); }
	QSize m_prevSize = QSize(0, 0);

	bool showImage() const { return m_showWhat & ImageCanvas::ImageObj; }
	bool showKarlsun() const { return m_showWhat & ImageCanvas::KarlsunObj; }
	bool showIndex() const { return m_showWhat & ImageCanvas::IndexStringObj; }

	QSize canvasSize() const
	{
		return m_prevSize;
	}
};


ImageCanvas::ImageCanvas(QWidget* parent)
	: QWidget(parent)
	, pImpl(new Internal(this))
{
	QPalette pal = palette();

	// set black background
	pal.setColor(QPalette::Background, Qt::darkGray);
	this->setAutoFillBackground(true);
	this->setPalette(pal);
}

QSize ImageCanvas::minimumSizeHint() const
{
	return QSize(640, 480);
}

//QSize ImageCanvas::sizeHint() const
//{
//	return QSize(640, 480);
//}

void ImageCanvas::resetCanvas()
{
	//pImpl->m_KarlsunBrush = QBrush //keep this
	pImpl->m_binImages.clear();
	pImpl->m_binQImages.clear();
	pImpl->m_karlsuns.clear();
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

void ImageCanvas::setCanvasSize(QSize inSize)
{
	if (pImpl->m_prevSize == inSize)
		return;

	const int padding = pImpl->m_canvasPadding;
	//inSize.setHeight(inSize.height() + padding * 2);
	//inSize.setWidth(inSize.width() + padding * 2);

	pImpl->m_prevSize = inSize;
	this->resize(pImpl->m_prevSize + QSize(padding*2, padding*2));
	update();
}

void ImageCanvas::setBinImages(std::vector<BinImagePtr> binImages)
{
	pImpl->m_binImages = binImages;
	pImpl->m_binQImages.clear();
	pImpl->m_karlsuns.clear();

	for (auto ptr : binImages)
	{
		QImage buf = ptr->imagePtr->toQImage().copy();
		buf.setOffset(ptr->result.topLeft());
		pImpl->m_binQImages.push_back(buf);
		pImpl->m_karlsuns.push_back(ptr->karlsun);
	}
	
	update();
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
	//QImage image = pImpl->m_image.copy();
	auto const& image = pImpl->m_binQImages;
	auto const& rects = pImpl->m_karlsuns;
	auto const& rectBrush = pImpl->m_KarlsunBrush;

	const QPoint canvasPadding = pImpl->canvasPadding();
	const QSize curSize = pImpl->canvasSize();

	QPainter painter(this);
	QPixmap imgBg(curSize);
	imgBg.fill();
	painter.drawPixmap(canvasPadding, imgBg);

	if (!image.empty() && pImpl->showImage())
	{
		for(auto const& qimg : image)
			painter.drawImage(QRect(qimg.offset() + canvasPadding, qimg.size()), qimg);
	}

	if (!rects.empty() && pImpl->showKarlsun())
	{
		for (auto const& karlsun : rects)
		{
			const QRect rect(karlsun.rect.topLeft() + canvasPadding, karlsun.rect.bottomRight() + canvasPadding);
			////TODO : apply color
			//QBrush rectBrush;
			//rectBrush.setColor(karlsun.style.color);
			//painter.setBrush(rectBrush);
			painter.drawRoundedRect(rect, karlsun.style.roundPixel, karlsun.style.roundPixel);
		}
	}

	if (!rects.empty() && pImpl->showIndex())
	{
		//set overall text properties;
		QFont font;
		font.setPointSize(30);
		painter.setFont(font);
		painter.setPen(Qt::red);

		for (auto const& karlsun : rects)
			painter.drawText(karlsun.rect.topLeft() + canvasPadding + QPoint(10, 50), QString("[%1]").arg(karlsun.imageIndex));
	}
}
