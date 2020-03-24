
#include "ImageObjectExample.h"

#include "BinpackMainWindow.h"
#include "CanvasTestWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
	if(0)
		ImageObjectExample ex;

	QApplication app(argc, argv);
	QMainWindow* mainWindow =
		//new CanvasTestWindow
		new BinpackMainWindow
		;
	mainWindow->show();

	return app.exec();
} 
