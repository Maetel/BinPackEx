#pragma once

#include "ImageCanvas.h"
#include "Karlsun.h"
#include <QMainWindow>

class BinpackMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	BinpackMainWindow(bool isDevMode = false);
	~BinpackMainWindow();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	//methods called from child windows
public:
	enum ReceiverType { CanvasResizer, ImageRemover, KarlsunSetter, DPISetter, ReceiverMaxNum };
	// call from receivers
	void setCanvasSize(QSize resultSize);
	void setRemoveImages(std::vector<int> const indicesToRemove);
	void setGlobalKarlsunStyle(KarlsunStyle);
	void setDPI(int);
	QSize canvasSize() const;
	KarlsunStyle karlsunStyle() const;
	int DPI() const;

	// call from canvas
	void callCanvasResize();
	void callSetGlobalKarlsunStyle();
	void callSetDPI();
	void callReset();
	void callImageRemove(std::vector<BinImagePtr> image2remove);

	//events
protected:
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;
	virtual void showEvent(QShowEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;

protected:
	class PImpl;
	PImpl* pImpl = 0;
	ImageCanvas* m_canvas = 0;
};