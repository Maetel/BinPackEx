#include "BinpackMainWindow.h"

//file IO
#include <QFileDialog>
#include <QPainter>
#include <QPdfWriter>

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
	KarlsunStyle globalKarlsunStyle = KarlsunStyle::DefaultStyle();
	int resultImageDPI = 300;
	int resultImageQuality = 100; // from 0 ~ 100

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

	void setGlobalKarlsunStyle()
	{
		qDebug() << "setKarlsunStyles";
		if (!imageManager.isAble())
		{
			qWarning() << "image mgr is not able to update karlsun";
			return;
		}

		for (auto ptr : imageManager.binImages)
		{
			ptr->updateKarlsun(
				globalKarlsunStyle.offset,
				globalKarlsunStyle.roundPixel,
				globalKarlsunStyle.color
			);
		}

		sendBinImages2Canvas();
	}

	void setKarlsunStyles(std::vector<KarlsunStyle> styles = std::vector<KarlsunStyle>())
	{
		qDebug() << "setKarlsunStyles";
		if (!imageManager.isAble())
		{
			qWarning() << "image mgr is not able to update karlsun";
			return;
		}

		// TODO : get style from GUI
		//preset for debug

		const auto imageCount = imageManager.imageCount();

		//if empty, put global style
		if (styles.empty())
			styles.emplace_back(KarlsunStyle::DefaultStyle());

		if (int styleCount = (int)styles.size(); styleCount < imageCount)
		{
			//duplicate last style
			for (int idx = styleCount; idx < imageCount; ++idx)
				styles.emplace_back(styles.back());
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

	inline BaseReceiver* selectReceiver(ReceiverType recType)
	{
		switch (recType)
		{
		case ReceiverType::CanvasResizer:
			return new SizeReceiver(Owner);
			break;
		case ReceiverType::DPISetter:
			return new DPIReceiver(Owner);
			break;
		case ReceiverType::ImageRemover:
			return new RemoveIndexReceiver(Owner, imageManager.imageCount());
			break;
		case ReceiverType::KarlsunSetter:
			return new KarlsunStyleReceiver(Owner);
			break;
		}
		return nullptr;
	}

	void popReceiver(ReceiverType recType)
	{
		static BaseReceiver* receiver = 0;
		util::HandyDelete(receiver);

		if (recType == ReceiverType::ImageRemover)
		{
			if (const int imageCount = imageManager.imageCount(); !imageCount)
			{
				Notify(KorStr("이미지 제거"), KorStr("제거할 이미지가 없습니다"));
				return;
			}
		}

		receiver = selectReceiver(recType);
		receiver->show();
	}

	void popKarlsunStyleSetter()
	{
		qDebug() << "Popping canvas resizer";
		static KarlsunStyleReceiver* receiver = 0;
		util::HandyDelete(receiver);
		receiver = new KarlsunStyleReceiver(Owner);
		receiver->show();
	}

	void popDPISetter()
	{
		qDebug() << "Popping DPI setter";
		static DPIReceiver* receiver = 0;
		util::HandyDelete(receiver);
		receiver = new DPIReceiver(Owner);
		receiver->show();
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
			Notify(KorStr("이미지 제거"), KorStr("제거할 이미지가 없습니다"));
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
			Notify(KorStr("자동 네스팅"), KorStr("네스팅 실패(이미지가 없습니다)"));
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
			setGlobalKarlsunStyle();
			sendBinImages2Canvas();
			//setKarlsunStyles();
			return true;
		}
		else
		{
			Notify(KorStr("자동 네스팅"), KorStr("네스팅에 실패했습니다"));
			return false;
		}

		return false;
	}

	void removeImages(std::vector<int> const& indices)
	{
		imageManager.storeCurState();
		if (imageManager.removeBinImage(indices))
		{
			Notify(KorStr("이미지 제거"), QString("%1%2").arg(indices.size()).arg(KorStr("개의 이미지가 제거되었습니다")));
			if (imageManager.imageCount())
			{
				if (!tryBinPack())
				{
					qDebug() << "Failed bin packing";
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
			Notify(KorStr("이미지 제거"), KorStr("이미지 제거 실패"));
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
		QAction* karlsunStyleAct = 0;
		QAction* setDPIAct = 0;
		QAction* removeImageAct = 0;
	};
	ControlToolbar controlToolbar;

	void createControlToolbar()
	{
		const auto ControlToolbarArea = Qt::ToolBarArea::TopToolBarArea;
		auto& ca = controlToolbar;

		ca.controlToolbar = new QToolBar(Owner);

		//file actions
		ca.openFileAct = new QAction(KorStr("이미지 열기"));
		util::actionPreset(ca.openFileAct, true, false, false);
		ca.controlToolbar->addAction(ca.openFileAct);

		ca.saveImageAct = new QAction(KorStr("결과 이미지 저장"));
		util::actionPreset(ca.saveImageAct, true, false, false);
		ca.controlToolbar->addAction(ca.saveImageAct);
		//!file actions

		ca.controlToolbar->addSeparator();

		//view actions
		ca.showImgAct = new QAction(KorStr("이미지 보기"));
		util::actionPreset(ca.showImgAct, true, true, true);
		ca.controlToolbar->addAction(ca.showImgAct);

		ca.showKsAct = new QAction(KorStr("칼선 보기"));
		util::actionPreset(ca.showKsAct, true, true, true);
		ca.controlToolbar->addAction(ca.showKsAct);

		ca.showImgIdxAct = new QAction(KorStr("번호 보기"));
		util::actionPreset(ca.showImgIdxAct, true, true, false);
		ca.controlToolbar->addAction(ca.showImgIdxAct);
		//!view actions

		ca.controlToolbar->addSeparator();


		//!control actions
		ca.keepPrevAct = new QAction(KorStr("이미지 추가 모드"));
		util::actionPreset(ca.keepPrevAct, true, true, keepPreviousImage);
		ca.controlToolbar->addAction(ca.keepPrevAct);

		ca.resetAct = new QAction(KorStr("리셋"));
		util::actionPreset(ca.resetAct, true, false, false);
		ca.controlToolbar->addAction(ca.resetAct);

		ca.canvasResizeAct = new QAction(KorStr("캔버스 사이즈"));
		util::actionPreset(ca.canvasResizeAct, true, false, false);
		ca.controlToolbar->addAction(ca.canvasResizeAct);

		ca.karlsunStyleAct = new QAction(KorStr("칼선 설정"));
		util::actionPreset(ca.karlsunStyleAct, true, false, false);
		ca.controlToolbar->addAction(ca.karlsunStyleAct);

		ca.setDPIAct = new QAction(KorStr("DPI 설정"));
		util::actionPreset(ca.setDPIAct, true, false, false);
		ca.controlToolbar->addAction(ca.setDPIAct);

		ca.removeImageAct = new QAction(KorStr("이미지 제거"));
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
		QLabel* sizeInMmLabel = 0;
		QLabel* itemCountLabel = 0;
		QLabel* outputDPILabel = 0;

		QWidget* karlsunInfoWidget = 0;
		QLabel* karlsunOffsetLabel = 0;
		QLabel* karlsunRoundingLabel = 0;
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

	void saveResults()
	{
		saveResultImage();
		savePDF();

		Notify(KorStr("결과 저장"), KorStr("저장이 완료되었습니다"));
	}

	void savePDF()
	{
		if (!finalImage)
		{
			Notify(KorStr("이미지 저장"), KorStr("저장할 이미지가 없습니다"));
			return;
		}

		const auto f = QFileDialog::getSaveFileName(Owner, KorStr("칼선 저장"), "", "PDF (*.pdf)");
		if (f.isEmpty())
			return;

		auto images = imageManager.binImages;
		std::vector<Karlsun> karlsuns;

		for (BinImagePtr ptr : images)
			karlsuns.push_back(ptr->karlsun);

		const int wid = finalImage->width(), hi = finalImage->height();
		
		QPdfWriter pdfWriter(f);
		pdfWriter.setPdfVersion(QPdfWriter::PdfVersion::PdfVersion_1_4);
		pdfWriter.setResolution(resultImageDPI);
		pdfWriter.setPageSizeMM({util::px2mm(wid, resultImageDPI), util::px2mm(hi, resultImageDPI)});
		QMarginsF pageMargin(0, 0, 0, 0);
		pdfWriter.setPageMargins(pageMargin);
		
		QPainter painter(&pdfWriter);
		QPen pen(Qt::black);
		pen.setWidthF(1.5);
		painter.setPen(pen);

		for (auto const& karlsun : karlsuns)
			painter.drawRoundedRect(karlsun.rect, karlsun.style.roundPixel, karlsun.style.roundPixel);
	}

	void saveResultImage()
	{
		if (!finalImage)
		{
			Notify(KorStr("이미지 저장"), KorStr("저장할 이미지가 없습니다"));
			return;
		}

		const auto f = QFileDialog::getSaveFileName(Owner, KorStr("이미지 저장"), "", "Jpg image (*.jpg)");
		if (f.isEmpty())
			return;

		resultImageQuality = std::clamp(resultImageQuality, 0, 100);
		finalImage->save(f, resultImageQuality, resultImageDPI);
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
		it.sizeInMmLabel = new QLabel(Owner);
		it.itemCountLabel = new QLabel(Owner);
		it.outputDPILabel = new QLabel(Owner);

		auto* karlsunInfoLayout = new QVBoxLayout;
		it.karlsunInfoWidget = new QWidget(Owner);
		it.karlsunOffsetLabel = new QLabel(Owner);
		it.karlsunRoundingLabel = new QLabel(Owner);

		canvasInfoLayout->addWidget(it.canvasInfoTitleLabel);
		canvasInfoLayout->addWidget(it.outputDPILabel);
		canvasInfoLayout->addWidget(it.widthLabel);
		canvasInfoLayout->addWidget(it.heightLabel);
		canvasInfoLayout->addWidget(it.sizeInMmLabel);
		canvasInfoLayout->addWidget(it.itemCountLabel);
		it.canvasInfoWidget->setLayout(canvasInfoLayout);
		karlsunInfoLayout->addWidget(it.karlsunOffsetLabel);
		karlsunInfoLayout->addWidget(it.karlsunRoundingLabel);
		it.karlsunInfoWidget->setLayout(karlsunInfoLayout);

		it.infoToolbar->addWidget(it.canvasInfoWidget);
		it.infoToolbar->addSeparator();
		it.infoToolbar->addWidget(it.karlsunInfoWidget);

		Owner->addToolBar(InfoToolbarArea, it.infoToolbar);
		updateInfoToolbar();
		// !Info toolbar
	}

	void updateInfoToolbar()
	{
		auto& it = infoToolbar;

		it.canvasInfoTitleLabel->setText(KorStr("캔버스 정보"));
		it.widthLabel->setText(QString("%1%2px").arg(KorStr("가로 : ")).arg(canvasSize.width()));
		it.heightLabel->setText(QString("%1%2px").arg(KorStr("세로 : ")).arg(canvasSize.height()));
		it.itemCountLabel->setText(QString("%1%2").arg(KorStr("이미지 개수 : ")).arg(imageManager.imageCount()));
		it.outputDPILabel->setText(QString("%1%2").arg("DPI : ").arg(resultImageDPI));

		const double widInMm = util::px2mm(canvasSize.width(), resultImageDPI);
		const double hiInMm = util::px2mm(canvasSize.height(), resultImageDPI);
		it.sizeInMmLabel->setText(QString("%3(mm) : %1 * %2").arg(QString::number(widInMm, 'f', 1)).arg(QString::number(hiInMm, 'f', 1)).arg(KorStr("사이즈")));

		const int dpi = resultImageDPI;
		const auto& st = globalKarlsunStyle;
		it.karlsunOffsetLabel->setText(QString("%1 : %2px (%3mm)").arg(KorStr("칼선 오프셋")).arg(st.offset).arg(QString::number(util::px2mm(st.offset, dpi), 'f', 1)));
		it.karlsunRoundingLabel->setText(QString("%1 : %2px (%3mm)").arg(KorStr("칼선 라운딩")).arg(st.roundPixel).arg(QString::number(util::px2mm(st.roundPixel, dpi), 'f', 1)));
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
		if (Notify<bool>(KorStr("캔버스 초기화"), KorStr("모든 내용이 제거됩니다")))
		{
			resetCanvas();
		}
	}

	void setConnections()
	{
		// control toolbar
		auto& ct = controlToolbar;
		connect(ct.openFileAct, &QAction::triggered, [=](bool c)	{ this->openImageFiles(); });
		connect(ct.saveImageAct, &QAction::triggered, [=](bool c)	{ this->saveResults(); });
		connect(ct.showImgAct, &QAction::triggered, [=](bool c)		{ this->showImage(c); });
		connect(ct.showKsAct, &QAction::triggered, [=](bool c)		{ this->showKarlsun(c); });
		connect(ct.showImgIdxAct, &QAction::triggered, [=](bool c)	{ this->showImageIndex(c); });
		connect(ct.keepPrevAct, &QAction::triggered, [=](bool c)	{ this->keepPreviousImage = c; });
		connect(ct.resetAct, &QAction::triggered, [=](bool c)		{ this->askResetCanvas(); });

		connect(ct.canvasResizeAct, &QAction::triggered, [=](bool c)	{ this->popReceiver(ReceiverType::CanvasResizer); });
		connect(ct.karlsunStyleAct, &QAction::triggered, [=](bool c)	{ this->popReceiver(ReceiverType::KarlsunSetter); });
		connect(ct.setDPIAct, &QAction::triggered, [=](bool c)			{ this->popReceiver(ReceiverType::DPISetter); });
		connect(ct.removeImageAct, &QAction::triggered, [=](bool c)		{ this->popReceiver(ReceiverType::ImageRemover); });
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
		pImpl->Notify(KorStr("이미지 제거"), KorStr("제거할 이미지가 없습니다"));
		return;
	}

	pImpl->removeImages(indices);
}
void BinpackMainWindow::setGlobalKarlsunStyle(KarlsunStyle setter)
{
	qDebug() << "Size set from setGlobalKarlsunStyle. Offset/Rounding : " << setter.offset << "/" << setter.roundPixel;
	pImpl->globalKarlsunStyle = setter;
	pImpl->setGlobalKarlsunStyle();
}

void BinpackMainWindow::setDPI(int DPI)
{
	if (DPI <= 0)
		DPI = 300;

	qDebug() << "Size set from setDPI. Input DPI : " << DPI;
	pImpl->resultImageDPI = DPI;
	pImpl->updateInfoToolbar();
}

QSize BinpackMainWindow::canvasSize() const
{
	return pImpl->canvasSize;
}

KarlsunStyle BinpackMainWindow::karlsunStyle() const
{
	return pImpl->globalKarlsunStyle;
}

int BinpackMainWindow::DPI() const
{
	return pImpl->resultImageDPI;
}

void BinpackMainWindow::callCanvasResize()
{
	pImpl->popCanvasResizer();
}

void BinpackMainWindow::callSetGlobalKarlsunStyle()
{
	pImpl->popKarlsunStyleSetter();
}

void BinpackMainWindow::callSetDPI()
{
	pImpl->popDPISetter();
}

void BinpackMainWindow::callReset()
{
	pImpl->askResetCanvas();
}

void BinpackMainWindow::callImageRemove(std::vector<BinImagePtr> image2remove)
{
	std::vector<int> indices;
	for (auto ptr : image2remove)
		indices.push_back(ptr->imageIndex);
	pImpl->removeImages(indices);
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
			pImpl->saveResults();
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

	m_canvas->keyPressEvent(event);
	
	//switch (event->key())
	//{
	//case Qt::Key_Escape:
	//	break;
	//case Qt::Key_Return:
	//case Qt::Key_Enter:
	//	break;
	//default:
	//	//QMainWindow::keyPressEvent(event);
	//	m_canvas->keyPressEvent(event);
	//}
}
