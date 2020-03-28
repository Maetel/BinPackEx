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

	Karlsun(QRect karlsun, KarlsunStyle karlsunStyle)
		: rect(karlsun)
		, style(karlsunStyle)
	{
	}

	Karlsun(QRect karlsun, int boundaryOffset, int roundPx = 0, QColor drawColor = Qt::red)
		: rect(karlsun)
		, style({boundaryOffset, roundPx, drawColor})
	{
	}
	
	void set(QRect karlsun, int boundaryOffset, int roundPx = 0, QColor drawColor = Qt::red)
	{
		rect = karlsun;
		style = KarlsunStyle{ boundaryOffset, roundPx, drawColor };
	}

	void set(QRect karlsun, KarlsunStyle karlsunStyle)
	{
		rect = karlsun;
		style = karlsunStyle;
	}

	bool isUsable() const
	{
		return (!rect.isEmpty() && style.isUsable());
	}
};

using KarlsunStyle = Karlsun::KarlsunStyle;