#pragma once

#include <vector>
#include <QString>


class ImagePathParser
{
public:
	enum ImageFormat { NONE=0, JPG = 1, BMP = 2, PNG = 4, ALL = JPG | BMP | PNG };
	ImageFormat currentFormat;
	ImagePathParser(ImageFormat format = ALL)
		: currentFormat(format)
	{
		updateSupportedFormats(format);
	}
	std::vector<QString> supportedFormats;

	void updateSupportedFormats(ImageFormat format)
	{
		currentFormat = format;
		supportedFormats.clear();
		if (format & JPG)
			supportedFormats.push_back(".jpg");
		if (format & BMP)
			supportedFormats.push_back(".bmp");
		if (format & PNG)
			supportedFormats.push_back(".png");
	}

	bool isSupportedFormat(QString path) const
	{
		for (auto format : supportedFormats)
		{
			if (path.endsWith(format, Qt::CaseInsensitive))
				return true;
		}
		return false;
	}
};