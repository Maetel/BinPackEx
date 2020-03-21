// ImageCanvas.h
#pragma once

#include <QWidget>
#include <QPen>

class ImageCanvas : public QWidget
{
	Q_OBJECT

public:	
	ImageCanvas(QWidget* parent = 0);
	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:

	void setImage(QImage const& image);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	class Internal;
	Internal* pImpl;
};