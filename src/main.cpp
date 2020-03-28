
#include "ImageObjectExample.h"

#include "BinpackMainWindow.h"
#include "CanvasTestWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
	if(0)
		ImageObjectExample ex;

	QApplication app(argc, argv);

	//if isDevMode, shows log window
	const bool isDevMode = app.arguments().contains("--diag", Qt::CaseInsensitive);


	QMainWindow* mainWindow = new BinpackMainWindow(isDevMode);
	mainWindow->show();

	return app.exec();
} 
