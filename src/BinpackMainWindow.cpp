#include "BinpackMainWindow.h"

//file IO
#include <QFileDialog>

//logging
#include "Logger.h"
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
#include <QValidator>
#include <QLineEdit>
#include <QLabel>

//events
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

//private classes
#include "Receivers.h"
#include "BinImageManager.h"
#include "Utils.h"

class BinpackMainWindow::PImpl
{
	BinpackMainWindow* Owner = 0;
public:
	PImpl(BinpackMainWindow* owner)
		: Owner(owner) 
	{
		//finalImage = std::make_shared<ImageDataRGB>(800, 600, RGB_WHITE);
	}
	~PImpl() {}

	ImagePathParser imagePathParser;
	BinImageManager imageManager;
	bool keepPreviousImage = false;
	ImageDataRGBPtr finalImage;
	QSize canvasSize{ 1600,1000 };

	template <typename T = void>
	T Notify(QString title, QString msg) {}

	template <>
	bool Notify(QString title, QString msg)
	{
		return
			QMessageBox::question(Owner, title, msg, QMessageBox::Yes | QMessageBox::No)
			== QMessageBox::Yes
			;
	}

	template <>
	void Notify(QString title, QString msg)
	{
		QMessageBox msgBox;
		msgBox.setText(title);
		msgBox.setInformativeText(msg);
		int ret = msgBox.exec();
	}

	void updateBinImage(QList<QUrl> const& fileList)
	{
		if (!keepPreviousImage)
		{
			imageManager.clear();
			resetCanvas();
		}

		for (auto url : fileList)
		{
			auto path = url.toLocalFile();

			if (imagePathParser.isSupportedFormat(path))
				imageManager.addImage(path);
		}
	}

	void updateCanvas()
	{
		Owner->m_canvas->update();
	}

	void resetCanvas()
	{
		qDebug() << "Canvas reset";
		imageManager.clear();
		Owner->m_canvas->resetCanvas();
		finalImage = 0;
		updateCanvas();
		updateInfoToolbar();
	}

	void drawImage()
	{
		if (finalImage && !finalImage->empty())
		{
			qDebug() << "Drawing image to canvas";
			QImage copied = finalImage->toQImage().copy();
			Owner->m_canvas->setImage(std::forward<QImage>(copied));
			updateCanvas();
		}
		else
		{
			qWarning() << "Failed to draw image";
		}
		
	}

	void drawKarlsun()
	{
		qDebug() << "Drawing karlsun to canvas";
		Owner->m_canvas->setKarlsun(imageManager.karlsuns());
		updateCanvas();
	}

	
	void showImageIndex(bool checked)
	{
		qDebug() << "show image index : " << checked;
		if (checked)
			Owner->m_canvas->showObejct(ImageCanvas::IndexStringObj);
		else
			Owner->m_canvas->hideObejct(ImageCanvas::IndexStringObj);
		updateCanvas();
	}

	void showKarlsun(bool checked)
	{
		qDebug() << "show Karlsun : " << checked;
		if (checked)
			Owner->m_canvas->showObejct(ImageCanvas::KarlsunObj);
		else
			Owner->m_canvas->hideObejct(ImageCanvas::KarlsunObj);
		updateCanvas();
	}

	void showImage(bool checked)
	{
		qDebug() << "show image : " << checked;
		if (checked)
			Owner->m_canvas->showObejct(ImageCanvas::ImageObj);
		else
			Owner->m_canvas->hideObejct(ImageCanvas::ImageObj);
		updateCanvas();
	}

	void handleDropEvent(QList<QUrl> const& fileList)
	{
		updateBinImage(fileList);
		if (tryBinPack())
		{
			// TODO : remember last state and restore if fails
		}
		updateCanvas();
		updateInfoToolbar();
	}

	void updateKarlsun(std::vector<KarlsunStyle> styles = std::vector<KarlsunStyle>())
	{
		qDebug() << "Updating Karlsun";
		if (!imageManager.isAble())
		{
			qWarning() << "image mgr is not able to update karlsun";
			return;
		}
			
		// TODO : get style from GUI
		//preset for debug

		const auto imageCount = imageManager.imageCount();
		const int offset = 20;
		const int roundPixel = 10;
		const QColor color = Qt::red;

		//while(styles.)
		if (styles.empty())
		{
			for (int idx = 0; idx < imageCount; ++idx)
				styles.emplace_back(KarlsunStyle{ offset, roundPixel, color });
		}

		int failureCount = 0;
		int index = 0;
		for (auto const& style : styles)
		{
			//const int index = style.imageIndex;
			if (auto img = imageManager.imageAt(index++))
				img->updateKarlsun(style.offset, style.roundPixel, style.color);
			else
				failureCount++;
		}
		
		if (failureCount)
		{
			qWarning() << QString("Failed to set %1 styles").arg(failureCount);
		}
		else
		{
			qDebug() << "Succeded to update karlsun.";
			drawKarlsun();
		}
			
	}
	void setKarlsunStyle()
	{

	}

	void popCanvasResizer()
	{
		qDebug() << "Popping canvas resizer";
		static SizeReceiver* receiver = 0;
		if (receiver)
			util::HandyDelete(receiver);
		receiver = new SizeReceiver(Owner);
		receiver->show();
	}

	bool tryBinPack()
	{
		auto& mgr = imageManager;
		mgr.setResultSize(canvasSize);

		if (!mgr.isAble())
		{
			Notify(__FUNCTION__, "BinPacking disabled (Image not loaded)");
			return false;
		}

		if (this->finalImage = mgr.binPack([this](QString msg) {Notify(__FUNCTION__, msg); }))
		{
			drawImage();
			updateKarlsun();
			setKarlsunStyle();
			return true;
		}
		else
		{
			Notify(__FUNCTION__, "Failed bin packing");
			return false;
		}

		return false;
	}

	void setCanvasSize(QSize size)
	{
		resetCanvas();

		qDebug() << "Canvas size set to " << size;
		canvasSize = size;

		//if(tryBinPack())
		//	updateCanvas();
		updateInfoToolbar();
	}

	void handleDevmode(bool isDevMode)
	{
		if (!isDevMode)
			return;

		//handle logger
		qInstallMessageHandler(Binpacklog);
		qDebug() << "[Dev mode start] Logger created";
		LogWindow* logger = new LogWindow(g_logEdit, Owner);
		logger->show();
	}

	struct ControlToolbar
	{
		QToolBar* controlToolbar = 0;
		QAction* saveImageAct = 0;
		QAction* showImgAct = 0;
		QAction* showKsAct = 0;
		QAction* showImgIdxAct = 0;
		QAction* keepPrevAct = 0;
		QAction* resetAct = 0;
		QAction* setResulSizeAct = 0;
	};
	ControlToolbar controlToolbar;

	void createControlToolbar()
	{
		const auto ControlToolbarArea = Qt::ToolBarArea::TopToolBarArea;
		auto& ca = controlToolbar;

		ca.controlToolbar = new QToolBar(Owner);

		ca.saveImageAct = new QAction("Save result image");
		util::actionPreset(ca.saveImageAct, true, false, false);
		ca.controlToolbar->addAction(ca.saveImageAct);

		ca.showImgAct = new QAction("View image");
		util::actionPreset(ca.showImgAct, true, true, true);
		ca.controlToolbar->addAction(ca.showImgAct);

		ca.showKsAct = new QAction("View karlsun");
		util::actionPreset(ca.showKsAct, true, true, true);
		ca.controlToolbar->addAction(ca.showKsAct);

		ca.showImgIdxAct = new QAction("View index");
		util::actionPreset(ca.showImgIdxAct, true, true, true);
		ca.controlToolbar->addAction(ca.showImgIdxAct);

		ca.keepPrevAct = new QAction("Keep previous images");
		util::actionPreset(ca.keepPrevAct, true, true, keepPreviousImage);
		ca.controlToolbar->addAction(ca.keepPrevAct);

		ca.resetAct = new QAction("Reset");
		util::actionPreset(ca.resetAct, true, false, false);
		ca.controlToolbar->addAction(ca.resetAct);

		ca.setResulSizeAct = new QAction("Set size");
		util::actionPreset(ca.setResulSizeAct, true, false, false);
		ca.controlToolbar->addAction(ca.setResulSizeAct);

		Owner->addToolBar(ControlToolbarArea, ca.controlToolbar);
	}

	struct InfoToolbar
	{
		QToolBar* infoToolbar = 0;

		QWidget* canvasInfoWidget = 0;
		QLabel* canvasInfoTitleLabel = 0;
		QLabel* widthLabel = 0;
		QLabel* heightLabel = 0;
		QLabel* itemCountLabel = 0;
	};
	InfoToolbar infoToolbar;

	void saveResultImage()
	{
		if (!finalImage)
		{
			Notify("Save image", "No image to save!");
			return;
		}

		const auto f = QFileDialog::getSaveFileName(Owner, "Save image", "", "Jpg image (*.jpg)");
		if (f.isEmpty())
			return;

		finalImage->save(f);
	}

	void createInfoToolbar()
	{
		const auto InfoToolbarArea = Qt::ToolBarArea::LeftToolBarArea;

		// Info toolbar
		auto& it = infoToolbar;
		it.infoToolbar = new QToolBar(Owner);

		auto* canvasInfoLayout = new QVBoxLayout;
		it.canvasInfoWidget = new QWidget(Owner);
		it.canvasInfoTitleLabel = new QLabel(Owner);
		it.widthLabel = new QLabel(Owner);
		it.heightLabel = new QLabel(Owner);
		it.itemCountLabel = new QLabel(Owner);
		canvasInfoLayout->addWidget(it.canvasInfoTitleLabel);
		canvasInfoLayout->addWidget(it.widthLabel);
		canvasInfoLayout->addWidget(it.heightLabel);
		canvasInfoLayout->addWidget(it.itemCountLabel);
		it.canvasInfoWidget->setLayout(canvasInfoLayout);
		it.infoToolbar->addWidget(it.canvasInfoWidget);
		it.infoToolbar->addSeparator();

		Owner->addToolBar(InfoToolbarArea, it.infoToolbar);
		updateInfoToolbar();
		// !Info toolbar
	}

	void updateInfoToolbar()
	{
		auto& it = infoToolbar;

		it.canvasInfoTitleLabel->setText("Canvas Info");
		it.widthLabel->setText(QString("%1%2").arg("Width : ").arg(canvasSize.width()));
		it.heightLabel->setText(QString("%1%2").arg("Height : ").arg(canvasSize.height()));
		it.itemCountLabel->setText(QString("%1%2").arg("Images : ").arg(imageManager.imageCount()));
	}

	void createCanvas()
	{
		// Canvas area
		Owner->m_canvas = new ImageCanvas(Owner);
		auto* scrollArea = new QScrollArea(Owner);
		scrollArea->setWidget(Owner->m_canvas);
		QLayout* layout = new QVBoxLayout;
		layout->addWidget(scrollArea);
		QWidget* mainWidget = new QWidget(Owner);
		mainWidget->setLayout(layout);
		Owner->setCentralWidget(mainWidget);
		// !Canvas area
	}

	void askResetCanvas()
	{
		qDebug() << "Asking reset canvas";
		if (Notify<bool>("Reset canvas\?", "All changes will be deleted"))
		{
			resetCanvas();
		}
	}

	void setConnections()
	{
		// control toolbar
		auto& ct = controlToolbar;
		connect(ct.saveImageAct, &QAction::triggered, [=](bool c)	{ this->saveResultImage(); });
		connect(ct.showImgAct, &QAction::triggered, [=](bool c)		{ this->showImage(c); });
		connect(ct.showKsAct, &QAction::triggered, [=](bool c)		{ this->showKarlsun(c); });
		connect(ct.showImgIdxAct, &QAction::triggered, [=](bool c)	{ this->showImageIndex(c); });
		connect(ct.keepPrevAct, &QAction::triggered, [=](bool c)	{ this->keepPreviousImage = c; });
		connect(ct.resetAct, &QAction::triggered, [=](bool c)		{ this->askResetCanvas(); });
		connect(ct.setResulSizeAct, &QAction::triggered, [=](bool c){ this->popCanvasResizer(); });
		//
	}

	public slots:
		void setResultSize(QSize resultSize)
		{
			canvasSize = resultSize;
		}
};

BinpackMainWindow::BinpackMainWindow(bool isDevMode)
	: pImpl(new PImpl(this))
{
	pImpl->handleDevmode(isDevMode);
	pImpl->createControlToolbar();
	pImpl->createInfoToolbar();
	pImpl->createCanvas();
	pImpl->setConnections();

	this->setAcceptDrops(true);

	
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
	return QSize(1024, 800);
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

	qDebug() << "Drop event occured";

	pImpl->handleDropEvent(fl);

}

void BinpackMainWindow::setCanvasSize(QSize size)
{
	//will be handled by SizeReceiver
	// check pImpl->popCanvasResizer()
	qDebug() << "Size set from SizeReceiver, size : " << size;
	pImpl->setCanvasSize(size);
}
