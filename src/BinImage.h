#pragma once

#include <QRect>
#include "ImageObject.h"

class BinImage
{
public:
	ImageDataRGBPtr imagePtr = 0;
	int imageIndex = -1; // Zero base index
	bool isFlipped = false;

	int offset = -1;
	QRect result; //rbp °á°ú
	QRect karlsun; //Ä®¼±

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

public:
	void updateKarlsun(int offset)
	{
		if (offset <= 0)
			return;
		this->offset = offset;
		karlsun = QRect{ result.x() + offset, result.y() + offset, result.width() - 2 * offset, result.height() - 2 * offset };
	}
};
using BinImagePtr = std::shared_ptr<BinImage>;