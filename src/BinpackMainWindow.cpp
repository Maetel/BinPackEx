#include "BinpackMainWindow.h"

#include <QDebug>
#include <QImage>
#include <QMenuBar>
#include <QFileDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

//drag and drops
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

//private classes
#include "BinImageManager.h"
#include "Utils.h"

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
	scrollArea->setFixedSize(800, 600);
	scrollArea->setWidget(m_canvas);
	auto* button = new QPushButton("Start bin pack", this);
	layout->addWidget(scrollArea);
	layout->addWidget(button);
	QWidget* mainWidget = new QWidget(this);
	mainWidget->setLayout(layout);
	setCentralWidget(mainWidget);

	this->setAcceptDrops(true);
	//connect(button, &QPushButton::clicked, this, &BinpackMainWindow::tryBinPack);
}
BinpackMainWindow::~BinpackMainWindow()
{
	util::HandyDelete(pImpl);
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
