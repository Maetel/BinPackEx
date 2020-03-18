#include "SomeWindow.h"
#include <QDebug>
#include <QImage>

SomeWindow::SomeWindow()
{
	connect(this, &SomeWindow::speak, this, &SomeWindow::handleSpeak);
}
SomeWindow::~SomeWindow()
{

}

void SomeWindow::handleSpeak()
{
	//std::cout << "Handle speak called!" << std::endl;
	QString str;
	QImage img;
	qDebug() << "Test qdebug";
}

//#include "SomeWindow.moc"