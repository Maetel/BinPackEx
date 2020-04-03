// ImageCanvas.h
#pragma once

#include <QWidget>
#include <QPen>
#include "Karlsun.h"

class BinImage;
using BinImagePtr = std::shared_ptr<BinImage>;

class ImageCanvas : public QWidget
{
	Q_OBJECT

public:
	enum CanvasObjectType {
		ImageObj = 1,
		KarlsunObj = 2,
		IndexStringObj = 4,
		ALL = ImageObj | KarlsunObj | IndexStringObj,
	};

public:	
	ImageCanvas(QWidget* parent = 0);
	QSize minimumSizeHint() const override;
	//QSize sizeHint() const override;

public slots:

	void showObejct(CanvasObjectType objType = ALL);
	void hideObejct(CanvasObjectType objType = ALL);
	void resetCanvas();
	
	void setCanvasSize(QSize);
	void setBinImages(std::vector<BinImagePtr> binimages);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	class Internal;
	Internal* pImpl;
};