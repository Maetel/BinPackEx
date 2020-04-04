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
#include <QKeyEvent>
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
	bool keepPreviousImage = true;
	ImageDataRGBPtr finalImage;
	QSize canvasSize{ 1600,1000 };

	template <typename T = void>
	T Notify(QString title, QString msg) {}

	//Question?
	template <>
	bool Notify(QString title, QString msg)
	{
		return
			QMessageBox::question(Owner, title, msg, QMessageBox::Yes | QMessageBox::No)
			== QMessageBox::Yes
			;
	}

	//Info/alert
	template <>
	void Notify(QString title, QString msg)
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle(title);
		msgBox.setText(msg);
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
		Owner->m_canvas->setCanvasSize(canvasSize);
		updateCanvas();
		updateInfoToolbar();
	}
	
	void sendBinImages2Canvas()
	{
		if (imageManager.isAble())
		{
			Owner->m_canvas->setCanvasSize(imageManager.resultSize);
			Owner->m_canvas->setBinImages(imageManager.binImages);
			updateCanvas();
		}
		else
		{
			qWarning() << "Failed to draw image";
		}

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

	void addImageList(QList<QUrl> const& fileList)
	{
		qDebug() << "adding images. Image count : " << fileList.size();

		imageManager.storeCurState();
		updateBinImage(fileList);
		if (!tryBinPack())
			imageManager.restoreLastState();
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
			sendBinImages2Canvas();
		}
			
	}
	void setKarlsunStyle()
	{

	}

	void popCanvasResizer()
	{
		qDebug() << "Popping canvas resizer";
		static SizeReceiver* receiver = 0;
		util::HandyDelete(receiver);
		receiver = new SizeReceiver(Owner);
		receiver->show();
	}

	void popImageRemover()
	{
		qDebug() << "Popping image remover";

		const int imageCount = imageManager.imageCount();
		if (!imageCount)
		{
			Notify("Remove image", "No images to remove");
			return;
		}

		static RemoveIndexReceiver* receiver = 0;
		util::HandyDelete(receiver);
		receiver = new RemoveIndexReceiver(Owner, imageCount);
		receiver->show();
	}

	bool tryBinPack()
	{
		auto& mgr = imageManager;
		mgr.setResultSize(canvasSize);

		if (!mgr.isAble())
		{
			Notify("Auto nesting", "Auto nesting disabled (Image not loaded)");
			resetCanvas();
			return false;
		}

		//if (this->finalImage = mgr.binPack([this](QString msg) {Notify(__FUNCTION__, msg); }))
#define _BINPACK_LOGGER_WARNING [](QString msg) { qWarning() << msg; }
#define _BINPACK_LOGGER_MUTE
#ifdef _DEBUG
#define BINPACK_LOGGER _BINPACK_LOGGER_WARNING
#else //Binpack logger is muted on release version
#define BINPACK_LOGGER _BINPACK_LOGGER_MUTE
#endif
		if (this->finalImage = mgr.binPack(BINPACK_LOGGER))
		{
			sendBinImages2Canvas();
			updateKarlsun();
			setKarlsunStyle();
			return true;
		}
		else
		{
			Notify("Auto nesting", "Failed to nest images");
			return false;
		}

		return false;
	}

	void removeImages(std::vector<int> const& indices)
	{
		imageManager.storeCurState();
		if (imageManager.removeBinImage(indices))
		{
			Notify("Remove image", "Image removed");
			if (imageManager.imageCount())
			{
				if (!tryBinPack())
				{
					imageManager.restoreLastState();
				}
				updateCanvas();
			}
			else
			{
				//no image to draw
				resetCanvas();
			}
			updateInfoToolbar();
		}
		else
		{
			Notify("Remove image", "Failed to remove images");
		}
	}

	void setCanvasSize(QSize size)
	{
		resetCanvas();

		qDebug() << "Canvas size set to " << size;
		canvasSize = size;
		Owner->m_canvas->setCanvasSize(canvasSize);

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
		QAction* openFileAct = 0;
		QAction* saveImageAct = 0;
		QAction* showImgAct = 0;
		QAction* showKsAct = 0;
		QAction* showImgIdxAct = 0;
		QAction* keepPrevAct = 0;
		QAction* resetAct = 0;
		QAction* canvasResizeAct = 0;
		QAction* removeImageAct = 0;
	};
	ControlToolbar controlToolbar;

	void createControlToolbar()
	{
		const auto ControlToolbarArea = Qt::ToolBarArea::TopToolBarArea;
		auto& ca = controlToolbar;

		ca.controlToolbar = new QToolBar(Owner);

		//file actions
		ca.openFileAct = new QAction("Open files");
		util::actionPreset(ca.openFileAct, true, false, false);
		ca.controlToolbar->addAction(ca.openFileAct);

		ca.saveImageAct = new QAction("Save result image");
		util::actionPreset(ca.saveImageAct, true, false, false);
		ca.controlToolbar->addAction(ca.saveImageAct);
		//!file actions

		ca.controlToolbar->addSeparator();

		//view actions
		ca.showImgAct = new QAction("View image");
		util::actionPreset(ca.showImgAct, true, true, true);
		ca.controlToolbar->addAction(ca.showImgAct);

		ca.showKsAct = new QAction("View karlsun");
		util::actionPreset(ca.showKsAct, true, true, true);
		ca.controlToolbar->addAction(ca.showKsAct);

		ca.showImgIdxAct = new QAction("View index");
		util::actionPreset(ca.showImgIdxAct, true, true, true);
		ca.controlToolbar->addAction(ca.showImgIdxAct);
		//!view actions

		ca.controlToolbar->addSeparator();


		//!control actions
		ca.keepPrevAct = new QAction("Keep previous images");
		util::actionPreset(ca.keepPrevAct, true, true, keepPreviousImage);
		ca.controlToolbar->addAction(ca.keepPrevAct);

		ca.resetAct = new QAction("Reset");
		util::actionPreset(ca.resetAct, true, false, false);
		ca.controlToolbar->addAction(ca.resetAct);

		ca.canvasResizeAct = new QAction("Set size");
		util::actionPreset(ca.canvasResizeAct, true, false, false);
		ca.controlToolbar->addAction(ca.canvasResizeAct);

		ca.removeImageAct = new QAction("Remove images");
		util::actionPreset(ca.removeImageAct, true, false, false);
		ca.controlToolbar->addAction(ca.removeImageAct);
		//!control actions

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

	void openImageFiles()
	{
		QFileDialog dialog(Owner);
		dialog.setDirectory(QDir::homePath());
		dialog.setFileMode(QFileDialog::ExistingFiles);
		dialog.setNameFilter(trUtf8("Images (*.jpg)"));
		QList<QUrl> fl;
		if (dialog.exec())
			fl = dialog.selectedUrls();

		if (fl.isEmpty())
		{
			qDebug() << "No valid image selected";
			return;
		}

		addImageList(fl);
	}

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
		connect(ct.openFileAct, &QAction::triggered, [=](bool c)	{ this->openImageFiles(); });
		connect(ct.saveImageAct, &QAction::triggered, [=](bool c)	{ this->saveResultImage(); });
		connect(ct.showImgAct, &QAction::triggered, [=](bool c)		{ this->showImage(c); });
		connect(ct.showKsAct, &QAction::triggered, [=](bool c)		{ this->showKarlsun(c); });
		connect(ct.showImgIdxAct, &QAction::triggered, [=](bool c)	{ this->showImageIndex(c); });
		connect(ct.keepPrevAct, &QAction::triggered, [=](bool c)	{ this->keepPreviousImage = c; });
		connect(ct.resetAct, &QAction::triggered, [=](bool c)		{ this->askResetCanvas(); });
		connect(ct.canvasResizeAct, &QAction::triggered, [=](bool c){ this->popCanvasResizer(); });
		connect(ct.removeImageAct, &QAction::triggered, [=](bool c){ this->popImageRemover(); });
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

	pImpl->addImageList(fl);

}

void BinpackMainWindow::setCanvasSize(QSize size)
{
	// Called by :
	// SizeReceiver::PImpl
	qDebug() << "Size set from SizeReceiver, size : " << size;
	if (pImpl->canvasSize == size)
		return;
	pImpl->setCanvasSize(size);
}
void BinpackMainWindow::setRemoveImages(std::vector<int> const indices)
{
	// Called by :
	// RemoveIndexReceiver::PImpl
	qDebug() << "image remove indices from RemoveIndexReceiver, size : " << indices.size();

	if (indices.empty())
	{
		pImpl->Notify("Remove image", "No image selected");
		return;
	}

	pImpl->removeImages(indices);
}

QSize BinpackMainWindow::canvasSize() const
{
	return pImpl->canvasSize;
}

void BinpackMainWindow::callCanvasResize()
{
	pImpl->popCanvasResizer();
}

void BinpackMainWindow::callReset()
{
	pImpl->askResetCanvas();
}

void BinpackMainWindow::showEvent(QShowEvent* event)
{
	pImpl->resetCanvas();
}

void BinpackMainWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->type() == QKeyEvent::KeyPress)
	{
		if (event->matches(QKeySequence::QKeySequence::Save))
		{
			pImpl->saveResultImage();
			return;
		}
		if (event->matches(QKeySequence::QKeySequence::Copy))
		{
			// copy image to clipboard
			return;
		}
		if (event->matches(QKeySequence::QKeySequence::Paste))
		{
			// paste image from clipboard
			return;
		}
	}

	switch (event->key())
	{
	case Qt::Key_Escape:
		break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
		break;
	default:
		QMainWindow::keyPressEvent(event);
	}
}
