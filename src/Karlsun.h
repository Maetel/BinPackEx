#pragma once

#include <QRect>
#include <QColor>

struct Karlsun
{
	//accessable variables
	QRect rect;
	int offset = -1;
	int roundPixel = -1;
	QColor color = Qt::red;

	Karlsun() = default;
	Karlsun(Karlsun const&) = default;
	Karlsun(Karlsun&&) = default;
	Karlsun& operator=(Karlsun const&) = default;
	Karlsun& operator=(Karlsun&&) = default;
	bool operator==(Karlsun const& rhs) const
	{
		return
			(rect == rhs.rect) &&
			(offset == rhs.offset) &&
			(roundPixel == rhs.roundPixel) &&
			(color == rhs.color)
			;
	}

	Karlsun(QRect karlsun, int boundaryOffset, int roundPx = 0, QColor drawColor = Qt::red)
		: rect(karlsun)
		, offset(boundaryOffset)
		, roundPixel(roundPx)
		, color(drawColor)
	{
	}
	
	void set(QRect karlsun, int boundaryOffset, int roundPx = 0, QColor drawColor = Qt::red)
	{
		rect = karlsun;
		offset = boundaryOffset;
		roundPixel = roundPx;
		color = drawColor;
	}

	bool isUsable() const
	{
		return !((rect.isEmpty()) || (offset <= 0) || (roundPixel < 0));
	}
};