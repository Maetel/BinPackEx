#include "BinpackMainWindow.h"
#include <QDebug>
#include <QImage>
#include <QMenuBar>
#include <QFileDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>

BinpackMainWindow::BinpackMainWindow()
{
	m_canvas = new ImageCanvas(this);

	QLayout* layout = new QVBoxLayout(this);

	auto* scrollArea = new QScrollArea(this);
	scrollArea->setFixedSize(800, 600);
	scrollArea->setWidget(m_canvas);
	auto* button = new QPushButton("Load image", this);
	layout->addWidget(scrollArea);
	layout->addWidget(button);
	QWidget* mainWidget = new QWidget(this);
	mainWidget->setLayout(layout);
	setCentralWidget(mainWidget);

	QMenuBar* menubar = new QMenuBar(this);
	QMenu* menu = new QMenu(this);
	auto* loadAction = new QAction("Load", this);
	menubar->addMenu(menu);
	menu->addAction(loadAction);
	this->setMenuBar(menubar);
	menubar->activateWindow();

	connect(loadAction, &QAction::triggered, this, &BinpackMainWindow::handleLoadImage);
	connect(button, &QPushButton::clicked, this, &BinpackMainWindow::handleLoadImage);
}
BinpackMainWindow::~BinpackMainWindow()
{

}

void BinpackMainWindow::handleLoadImage()
{
	QFileDialog dia;
	QString file = dia.getOpenFileName(this, "Load image", QString(), "image (*.jpg *.png *.bmp)");
	m_canvas->setImage(QImage(file));
	qDebug() << "Image loaded : " << file;
}

//#include "SomeWindow.moc"