// ImageCanvas.h
#pragma once

#include <QWidget>
#include <QPen>
#include "Karlsun.h"

class ImageCanvas : public QWidget
{
	Q_OBJECT

public:
	enum CanvasObjectType {
		ImageObj = 1,
		KarlsunObj = 2,
		ALL = ImageObj | KarlsunObj,
	};

public:	
	ImageCanvas(QWidget* parent = 0);
	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:

	void showObejct(CanvasObjectType objType = ALL);
	void hideObejct(CanvasObjectType objType = ALL);
	void resetCanvas();
	void setKarlsunBrush(QBrush brush);
	void setKarlsun(std::vector<Karlsun> const& karlsuns);
	void setImage(QImage const& image);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	class Internal;
	Internal* pImpl;
};