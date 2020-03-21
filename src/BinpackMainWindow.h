#pragma once

#include "ImageCanvas.h"
#include <QMainWindow>

class BinpackMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	BinpackMainWindow();
	~BinpackMainWindow();

public slots:
	void handleLoadImage();

protected:
	ImageCanvas* m_canvas = 0;
};