#include "BinpackMainWindow.h"

//file IO
#include <QFileDialog>

//logging
#include <QDebug>
#include <QMessageBox>

//UI components
#include <QMenuBar>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QCheckBox>
#include <QDockWidget>
#include <QToolBar>

//drag and drops
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

//private classes
#include "BinImageManager.h"
#include "Utils.h"

struct KarlsunStyle
{
	int imageIndex = -1;
	int offset = -1;
	int roundPixel = 0;
	QColor color = Qt::red;
};

class BinpackMainWindow::PImpl
{
	BinpackMainWindow* Owner = 0;
public:
	PImpl(BinpackMainWindow* owner) : Owner(owner) {}
	~PImpl() {}

	ImagePathParser imagePathParser;
	BinImageManager imageManager;

	void Notify(QString msg)
	{
		QMessageBox msgBox;
		msgBox.setText("Alert");
		msgBox.setInformativeText(msg);
		int ret = msgBox.exec();
	}

	void updateBinImage(QList<QUrl> const& fileList, bool keepPrevious = false)
	{
		if(!keepPrevious)
			imageManager.clear();

		for (auto url : fileList)
		{
			auto path = url.toLocalFile();

			if (imagePathParser.isSupportedFormat(path))
				imageManager.addImage(path);
		}
	}

	void resetCanvas()
	{
		Owner->m_canvas->resetCanvas();
	}

	void drawKarlsun()
	{
		Owner->m_canvas->setKarlsun(imageManager.karlsuns());
	}

	void showKarlsun(bool checked)
	{
		if (checked)
			Owner->m_canvas->showObejct(ImageCanvas::KarlsunObj);
		else
			Owner->m_canvas->hideObejct(ImageCanvas::KarlsunObj);
	}

	void showImage(bool checked)
	{
		if (checked)
			Owner->m_canvas->showObejct(ImageCanvas::ImageObj);
		else
			Owner->m_canvas->hideObejct(ImageCanvas::ImageObj);
	}

	void updateKarlsun()
	{
		if (!imageManager.isAble())
			return;

		//preset for debug
		const auto imageCount = imageManager.imageCount();
		const int offset = 20;
		const int roundPixel = 10;
		const QColor color = Qt::red;
		std::vector<KarlsunStyle> styles;
		for (int idx = 0; idx < imageCount; ++idx)
			styles.emplace_back(KarlsunStyle{ idx, offset, roundPixel, color });
		//!preset for debug


		int failureCount = 0;
		for (auto const& style : styles)
		{
			const int index = style.imageIndex;
			if (auto img = imageManager.imageAt(index))
				img->updateKarlsun(style.offset, style.roundPixel, style.color);
			else
				failureCount++;
		}
		
		if (failureCount)
			Notify(QString("Failed to set %1 styles").arg(failureCount));
		else
			drawKarlsun();
	}
	void setKarlsunStyle()
	{

	}

	void tryBinPack()
	{
		auto& mgr = imageManager;
		if (!mgr.isAble())
		{
			Notify("BinPacking disabled (Image not loaded)");
			return;
		}

		const int dst_wid = 1600, dst_hi = 1000;
		auto finalImage = mgr.binPack(dst_wid, dst_hi, [=](QString msg) {Notify(msg); });
		if (!finalImage)
		{
			Notify("Failed bin packing");
			return;
		}

		//this->m_canvas->setFixedSize(dst_wid, dst_hi);
		auto copied = finalImage->toQImage();
		Owner->m_canvas->setImage(copied.copy());
	}
};

BinpackMainWindow::BinpackMainWindow()
	: pImpl(new PImpl(this))
{
	m_canvas = new ImageCanvas(this);

	QLayout* layout = new QVBoxLayout(this);

	auto* scrollArea = new QScrollArea(this);
	//scrollArea->setFixedSize(800, 600);
	scrollArea->setWidget(m_canvas);
	layout->addWidget(scrollArea);

	//auto* iconDock = new QDockWidget(this);
	//iconDock->add

	auto* toolBar = new QToolBar(this);
	auto* drawKarlsunAct = new QAction("Draw Karlsun");
	util::actionPreset(drawKarlsunAct, true, false, false);
	auto* showImgAct = new QAction("Show image");
	util::actionPreset(showImgAct, true, true, true);
	auto* showKsAct = new QAction("Show karlsun");
	util::actionPreset(showKsAct, true, true, true);
	
	toolBar->addAction(drawKarlsunAct);
	toolBar->addAction(showImgAct);
	toolBar->addAction(showKsAct);
	

	QWidget* mainWidget = new QWidget(this);
	mainWidget->setLayout(layout);
	setCentralWidget(mainWidget);
	//this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, iconDock);
	this->addToolBar(Qt::ToolBarArea::LeftToolBarArea, toolBar);

	this->setAcceptDrops(true);
	connect(drawKarlsunAct, &QAction::triggered, [=](bool c) {handleKarlsun(); });
	connect(showImgAct, &QAction::triggered, [=](bool c) {showImage(c); });
	connect(showKsAct, &QAction::triggered, [=](bool c) {showKarlsun(c); });
}
BinpackMainWindow::~BinpackMainWindow()
{
	util::HandyDelete(pImpl);
}

QSize BinpackMainWindow::minimumSizeHint() const
{
	return QSize(800, 600);
}

QSize BinpackMainWindow::sizeHint() const
{
	return QSize(800, 600);
}

void BinpackMainWindow::showImage(bool checked)
{
	pImpl->showImage(checked);
}
void BinpackMainWindow::showKarlsun(bool checked)
{
	pImpl->showKarlsun(checked);
}

void BinpackMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept();
}



void BinpackMainWindow::dropEvent(QDropEvent* event)
{
	//file list
	auto fl = event->mimeData()->urls();
	if (fl.isEmpty())
		return;

	const bool keepPrev = false;
	pImpl->updateBinImage(fl, keepPrev);
	pImpl->tryBinPack();
}

void BinpackMainWindow::handleKarlsun()
{
	pImpl->updateKarlsun();
	pImpl->setKarlsunStyle();
}