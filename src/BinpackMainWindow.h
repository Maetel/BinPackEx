#pragma once

#include "ImageCanvas.h"
#include <QMainWindow>

class BinpackMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	BinpackMainWindow(bool isDevMode = false);
	~BinpackMainWindow();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	//https://evileg.com/en/post/240/
	// Drag event method
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	// Method for drop an object with data
	virtual void dropEvent(QDropEvent* event) override;

public slots:
	void showImage(bool checked);
	void showKarlsun(bool checked);
	void setCanvasSize(QSize resultSize);

protected:
	class PImpl;
	PImpl* pImpl = 0;
	ImageCanvas* m_canvas = 0;
};