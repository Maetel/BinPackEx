#pragma once

#include <QString>
#include <stack>
#include "BinImage.h"
#include "ImagePathParser.h"
#include "BinPacker.h"

class BinImageManager
{
public:
	ImagePathParser m_parser;
	std::unique_ptr<BaseBinPacker> binPacker = 0;

	using BinImages = std::vector<BinImagePtr>;
	BinImages binImages;

public:
	bool isAble()
	{
		return binPacker && !binImages.empty();
	}

	BinImageManager(BinPackAlgorithm algorithm = Guillotine)
	{
		createBinPacker(algorithm);
	}

	void createBinPacker(BinPackAlgorithm algorithm)
	{
		if (binPacker)
			binPacker = nullptr;

		switch (algorithm)
		{
		case Guillotine:
			binPacker = std::make_unique<BinPacker<Guillotine>>();
			break;
		default:
			break;
		}
	}

	ImageDataRGBPtr makeFinalImage(int dst_wid, int dst_hi)
	{
		auto retval = std::make_shared<ImageDataRGB>(dst_wid, dst_hi);
		for (auto binImg : binImages)
		{
			auto img = binImg->imagePtr;
			const auto startPoint = binImg->result.topLeft();
			if (!retval->drawSubImage(*img, startPoint.x(), startPoint.y(), binImg->isFlipped))
				return nullptr;
		}
		return retval;
	}

	template <typename FuncT>
	ImageDataRGBPtr binPack(int dst_wid, int dst_hi, FuncT&& logger)
	{
		ImageDataRGBPtr retval = 0;
		auto result = binPacker->run(dst_wid, dst_hi, binImages);

		QString err;
		//if error
		if (result)
		{
			err = QString(BinPackErrorToString.at(std::abs(result)));
			QString log = QString("Bin packing error. Error message : [%1] @[%2] @LINE[%3]").arg(err).arg(__FUNCTION__).arg(__LINE__);
			logger(log);
			return retval;
		}
		
		retval = makeFinalImage(dst_wid, dst_hi);

		return retval;
	}

	int imageCount() const { return (int)binImages.size(); }

	bool addImage(QString path)
	{
		if (!m_parser.isSupportedFormat(path))
			return false;

		auto img = std::make_shared<ImageDataRGB>();
		if (!img->load(path))
			return false;

		addImage(img, path);

		return false;
	}

	void addImage(ImageDataRGBPtr image, QString path = "")
	{
		binImages.push_back(std::make_shared<BinImage>(image, imageCount(), path));
	}

	//returns true if successfully removed
	bool removeBinImage(int index)
	{
		if (index >= imageCount() || index < 0)
			return false;

		binImages.erase(binImages.begin() + index);

		return true;
	}

	void clear()
	{
		//allocate new stack
		binImages = BinImages();
	}
};