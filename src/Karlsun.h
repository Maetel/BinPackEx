#pragma once

#include <QRect>
#include <QColor>


struct Karlsun
{
	struct KarlsunStyle
	{
		KarlsunStyle() = default;
		KarlsunStyle(KarlsunStyle const&) = default;
		KarlsunStyle(KarlsunStyle&&) = default;
		KarlsunStyle& operator=(KarlsunStyle const&) = default;
		KarlsunStyle& operator=(KarlsunStyle&&) = default;

		int offset = -1;
		int roundPixel = 0;
		QColor color = Qt::red;

		bool operator==(KarlsunStyle const& rhs) const
		{
			return
				(offset == rhs.offset) &&
				(roundPixel == rhs.roundPixel) &&
				(color == rhs.color)
				;
		}

		bool isUsable() const
		{
			return !(offset < 0 && roundPixel < 0);
		}
	};

	//accessable variables
	QRect rect;
	KarlsunStyle style;
	int imageIndex = -1;

	Karlsun() = default;
	Karlsun(Karlsun const&) = default;
	Karlsun(Karlsun&&) = default;
	Karlsun& operator=(Karlsun const&) = default;
	Karlsun& operator=(Karlsun&&) = default;
	bool operator==(Karlsun const& rhs) const
	{
		return
			(rect == rhs.rect) &&
			style == rhs.style
			;
	}

	Karlsun(int imgIndex, QRect karlsun, KarlsunStyle karlsunStyle)
		: rect(karlsun)
		, imageIndex(imgIndex)
		, style(karlsunStyle)
	{
	}

	Karlsun(int imgIndex, QRect karlsun, int boundaryOffset, int roundPx = 0, QColor drawColor = Qt::red)
		: rect(karlsun)
		, imageIndex(imgIndex)
		, style({boundaryOffset, roundPx, drawColor})
	{
	}
	
	void set(int imgIndex, QRect karlsun, int boundaryOffset, int roundPx = 0, QColor drawColor = Qt::red)
	{
		imageIndex = imgIndex;
		rect = karlsun;
		style = KarlsunStyle{ boundaryOffset, roundPx, drawColor };
	}

	void set(int imgIndex, QRect karlsun, KarlsunStyle karlsunStyle)
	{
		imageIndex = imgIndex;
		rect = karlsun;
		style = karlsunStyle;
	}

	bool isUsable() const
	{
		return (!rect.isEmpty() && imageIndex != -1 && style.isUsable());
	}
};

using KarlsunStyle = Karlsun::KarlsunStyle;