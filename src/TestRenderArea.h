// ImageObjectExample.h
#pragma once

#include <QWidget>
#include <QPen>

class TestRenderArea : public QWidget
{
	Q_OBJECT

public:
	enum Shape {
		Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc,
		Chord, Pie, Path, Text, Pixmap
	};

	TestRenderArea(QWidget* parent = 0);

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:

	void setShape(Shape shape);
	void setPen(const QPen& pen);
	void setBrush(const QBrush& brush);
	void setAntialiased(bool antialiased);
	void setTransformed(bool transformed);
	void setImage(QImage const& image);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	Shape shape;
	QPen pen;
	QBrush brush;
	bool antialiased;
	bool transformed;
	QPixmap pixmap;
};