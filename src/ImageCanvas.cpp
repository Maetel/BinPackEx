#include "BinpackMainWindow.h"
#include "Utils.h"
//#include "ImageCanvas.h"
#include <QPainter>
#include <QRectF>
#include <QTextItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QDebug>
#include <QPointer>
#include "BinImage.h"

class ImageCanvas::Internal
{
public:
	BinpackMainWindow* Owner = 0;

	Internal(BinpackMainWindow* owner)
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

	struct _EventState
	{
		_EventState() {}
		~_EventState() {}
		std::vector<BinImagePtr> selectedBinImages;

		std::vector<int> selectedIndices() { std::vector<int> retval; for (BinImagePtr ptr : selectedBinImages) { retval.push_back(ptr->imageIndex); } return retval; }
		void reset() { selectedBinImages.clear(); }
		bool notSelected() const { return selectedBinImages.empty(); };
		bool anySelected() const { return !selectedBinImages.empty(); };
		BinImagePtr oneSelected() const { if (selectedBinImages.size() == 1) return selectedBinImages.at(0); else return nullptr; }
		bool multipleSelected() const { return selectedBinImages.size() > 1; }
		std::vector<QRect> selectedRects() const 
		{ 
			std::vector<QRect> retval;
			for (auto ptr : selectedBinImages)
				retval.push_back(ptr->result);
			return retval;
		}
	};
	using EventState = std::shared_ptr<_EventState>;
	EventState m_eventState = std::make_shared<_EventState>();

	std::atomic_int m_canvasPadding = 20;
	QPoint canvasPadding() const { return QPoint(m_canvasPadding, m_canvasPadding); }
	QSize m_prevSize = QSize(0, 0);

	BinImagePtr findBinImageContaning(QPoint actualPos) const
	{
		for (auto ptr : m_binImages)
			if (ptr->result.contains(actualPos))
				return ptr;
		return BinImagePtr();
	}

	bool showImage() const { return m_showWhat & ImageCanvas::ImageObj; }
	bool showKarlsun() const { return m_showWhat & ImageCanvas::KarlsunObj; }
	bool showIndex() const { return m_showWhat & ImageCanvas::IndexStringObj; }
	bool showSelectedImage() const { return m_showWhat & ImageCanvas::SelectedImageObj; }

	void updateSelected(QPoint actualPos, bool keepPreviousSelected = false)
	{
		auto& state = m_eventState;

		if (state->notSelected())
		{
			state->reset();
			if (auto binImg = findBinImageContaning(actualPos))
				state->selectedBinImages.push_back(binImg);
		}
		else //is rect selected already
		{
			if (auto preSelected = state->oneSelected())
			{
				if (preSelected->result.contains(actualPos))
				{
					if(!keepPreviousSelected)
						state->reset();
					return;
				}
				else if (auto otherImageFound = findBinImageContaning(actualPos))
				{
					state->reset();
					state->selectedBinImages.push_back(otherImageFound);
				}
				else //clicked on white canvas
				{
				}
			}
			else //multiple images selected
			{

			}
		}
	}

	void update() { Owner->update(); }

	void handleClick(QMouseEvent* mousePos, bool isLeft, bool isRight)
	{
		const auto globalPos= mousePos->globalPos();
		const auto localPos = mousePos->pos();

		//when clicked on gray boundary area
		if (!canvasRect().contains(localPos))
		{
			if(isLeft)
			{
				m_eventState->reset();
			}
			if (isRight)
			{
				static QMenu* dropMenu;
				if (!dropMenu)
				{
					dropMenu = new QMenu(Owner);
					static QAction* resizeAction = new QAction(KorStr("사이즈 설정"));
					static QAction* resetAction = new QAction(KorStr("리셋"));
					dropMenu->addAction(resizeAction);
					dropMenu->addAction(resetAction);
					Owner->connect(resizeAction, &QAction::triggered, [=](bool c) { qDebug() << "Resize action"; Owner->callCanvasResize(); });
					Owner->connect(resetAction, &QAction::triggered, [=](bool c) { qDebug() << "Reset canvas action"; Owner->callReset(); });
				}
				dropMenu->popup(globalPos);
			}
			return;
		}
		
		const QPoint actualPos = localPos - canvasPadding();
		
		if (isLeft)
		{
			if (m_eventState->multipleSelected())
				m_eventState->reset();
			updateSelected(actualPos);
		}
		if (isRight)
		{
			if (m_eventState->notSelected() || m_eventState->oneSelected())
			{
				const bool keepPrevious = true;
				updateSelected(actualPos, keepPrevious);
			}

			//open menu
			static QMenu* dropMenu;
			if (!dropMenu)
			{
				dropMenu = new QMenu(Owner);
				static QAction* remove = new QAction(KorStr("이미지 제거"), dropMenu);
				dropMenu->addAction(remove);
				Owner->connect(remove, &QAction::triggered, [=](bool c) 
				{ 
					qDebug() << "Remove image action from image canvas"; 
					//Owner->callImageRemove(m_eventState->selectedBinImages);
					Owner->setRemoveImages(m_eventState->selectedIndices());
				});
			}
			dropMenu->popup(globalPos);
			
		}
		update();
	}

	QRect canvasRect(bool includePadding = false) const
	{
		return QRect(
			(includePadding ? QPoint(0, 0) : canvasPadding()),
			canvasSize(includePadding)
		);
	}

	QSize canvasSize(bool includePadding = false) const
	{
		return m_prevSize + (includePadding ? QSize(m_canvasPadding, m_canvasPadding) : QSize(0,0));
	}

	void clearAll()
	{
		m_binImages.clear();
		m_binQImages.clear();
		m_eventState->reset();
		m_karlsuns.clear();
	}
};


ImageCanvas::ImageCanvas(BinpackMainWindow* parent)
	: pImpl(new Internal(parent))	// Allocate internal first, than QWidget
	, QWidget(parent)
{
	QPalette pal = palette();

	// set black background
	pal.setColor(QPalette::Background, Qt::darkGray);
	this->setAutoFillBackground(true);
	this->setPalette(pal);

	hideObejct(CanvasObjectType::IndexStringObj);
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
	pImpl->clearAll();
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
	pImpl->m_prevSize = inSize;
	this->resize(pImpl->m_prevSize + QSize(padding*2, padding*2));

	//clear when resizing
	pImpl->clearAll();

	update();
}

void ImageCanvas::setBinImages(std::vector<BinImagePtr> binImages)
{
	pImpl->m_binImages = binImages;
	pImpl->m_binQImages.clear();
	pImpl->m_karlsuns.clear();
	pImpl->m_eventState->reset();

	for (BinImagePtr ptr : binImages)
	{
		//QImage buf = ptr->eval()->toQImage().copy();
		QImage buf = ptr->imagePtr->toQImage().copy();
		buf.setOffset(ptr->result.topLeft());
		pImpl->m_binQImages.push_back(buf);
		pImpl->m_karlsuns.push_back(ptr->karlsun);
	}
	
	update();
}


void ImageCanvas::keyPressEvent(QKeyEvent* event)
{
	if (event->type() != QKeyEvent::KeyPress)
		return;

	if (event->matches(QKeySequence::QKeySequence::SelectAll))
	{
		qDebug() << "Ctrl + A on canvas";
		pImpl->m_eventState->selectedBinImages = pImpl->m_binImages;
		update();
		return;
	}

	switch (event->key())
	{
	case Qt::Key_Escape:
		qDebug() << "Esc pressed from canvas";
		pImpl->m_eventState->reset();
		break;
	case Qt::Key_Return:	// main enter key
	case Qt::Key_Enter:		// numpad enter key
		qDebug() << "Enter pressed from canvas";
		break;
	default:
		QWidget::keyPressEvent(event);
	}

	update();
}

void ImageCanvas::mousePressEvent(QMouseEvent* event)
{
	bool leftClicked = event->button() == Qt::LeftButton;
	bool rightClicked = event->button() == Qt::RightButton;
	
	if(leftClicked || rightClicked)
		pImpl->handleClick(event, leftClicked, rightClicked);
	update();
}

void ImageCanvas::paintEvent(QPaintEvent* event)
{
	//QImage image = pImpl->m_image.copy();
	auto const& image = pImpl->m_binQImages;
	auto const& rects = pImpl->m_karlsuns;
	auto const& rectBrush = pImpl->m_KarlsunBrush;

	const QPoint canvasPadding = pImpl->canvasPadding();
	auto paddedRect = [canvasPadding](QRect input)->QRect
	{
		return QRect(input.topLeft() + canvasPadding, input.bottomRight() + canvasPadding);
	};
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
			auto prevPen = painter.pen();
			
			painter.setPen(karlsun.style.color);
			painter.drawRoundedRect(paddedRect(karlsun.rect), karlsun.style.roundPixel, karlsun.style.roundPixel);

			painter.setPen(prevPen);
		}
	}

	if (!rects.empty() && pImpl->showIndex())
	{
		auto prevPen = painter.pen();
		auto prevFont = painter.font();

		//set overall text properties;
		QFont font;
		font.setPointSize(30);
		painter.setFont(font);
		painter.setPen(Qt::red);

		for (auto const& karlsun : rects)
			painter.drawText(karlsun.rect.topLeft() + canvasPadding + QPoint(10, 50), QString("[%1]").arg(karlsun.imageIndex));

		painter.setFont(prevFont);
		painter.setPen(prevPen);
	}

	//show image boundary when clicked
	if (auto state = pImpl->m_eventState)
	{
		if (!pImpl->showSelectedImage() || state->notSelected())
			return;
		auto prevPen = painter.pen();
		
		QPen boundaryPen;
		boundaryPen.setStyle(Qt::PenStyle::DashLine);
		boundaryPen.setColor(Qt::darkBlue);
		boundaryPen.setWidth(2);
		painter.setPen(boundaryPen);
		for (auto rect : state->selectedRects())
			painter.drawRect(paddedRect(rect));

		painter.setPen(prevPen);
	}
}
