#pragma once

#include <QRect>
#include "ImageObject.h"
#include "Karlsun.h"

class BinImage
{
public:
	using Ptr = std::shared_ptr<BinImage>;
	using ConstPtr = const std::shared_ptr<BinImage>;
	ImageDataRGBPtr imagePtr = 0;
	int imageIndex = -1; // Zero base index
	bool isFlipped = false;

	QRect result; //rbp °á°ú
	Karlsun karlsun; //Ä®¼±

	//metadata
	QString path;

	BinImage() {}
	BinImage(ImageDataRGBPtr image, int index, QString filePath)
		: imagePtr(image)
		, imageIndex(index)
		, path(filePath)
	{
	}
	~BinImage() {}

	bool operator==(BinImage const& rhs)
	{
		//if not initialized, return false
		if (imageIndex == -1 || rhs.imageIndex == -1)
			return false;

		//compare image index only
		return imageIndex == rhs.imageIndex;
	}

public:
	void updateKarlsun(int offset, int roundPx = 0, QColor drawColor = Qt::red)
	{
		if (offset <= 0 || roundPx < 0)
			return;
		
		karlsun = Karlsun(
			this->imageIndex,
			QRect{
				result.x() + offset,
				result.y() + offset,
				result.width() - 2 * offset,
				result.height() - 2 * offset
			},
			offset,
			roundPx,
			drawColor
		);
	}
};
using BinImagePtr = std::shared_ptr<BinImage>;