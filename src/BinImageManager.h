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
	QSize resultSize{ -1,-1 };

public:

	void setResultSize(QSize size)
	{
		resultSize = size;
	}

	void setResultSize(int dst_wid, int dst_hi)
	{
		resultSize = QSize{ dst_wid, dst_hi };
	}

	bool isResultSizeReady() const
	{
		return resultSize.width() > 0 && resultSize.height() > 0;
	}

	bool isAble() const
	{
		return binPacker && !binImages.empty() && isResultSizeReady();
	}

	BinImages& images() { return binImages; }
	BinImages const& images() const { return binImages; }
	BinImagePtr& imageAt(int index)
	{
		if (index >= imageCount())
			return BinImagePtr();
		return binImages.at(index);
	}
	BinImagePtr const& imageAt(int index) const
	{
		if (index >= imageCount())
			return BinImagePtr();
		return binImages.at(index);
	}

	std::vector<Karlsun> karlsuns() const
	{
		std::vector<Karlsun> retval;

		for (auto img : binImages)
			if (auto k = img->karlsun; k.isUsable())
				retval.push_back(k);
		
		return retval;
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

	ImageDataRGBPtr makeFinalImage(VectorRGB background = VectorRGB::Constant(3, 1, 255))
	{
		ImageDataRGBPtr retval = 0;
		if (!isResultSizeReady())
			return retval;

		const int dst_wid = resultSize.width();
		const int dst_hi = resultSize.height();

		retval = std::make_shared<ImageDataRGB>(dst_wid, dst_hi, background);
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
	ImageDataRGBPtr binPack(FuncT&& logger)
	{
		ImageDataRGBPtr retval = 0;

		if (!isResultSizeReady())
			return retval;

		const int dst_wid = resultSize.width();
		const int dst_hi = resultSize.height();

		BinPackError error = binPacker->run(dst_wid, dst_hi, binImages);

		if (error)
		{
			QString errorLog = QString(BinPackErrorToString.at(std::abs(error)));
			QString log = QString("Bin packing error. Error message : [%1] @[%2] @LINE[%3]").arg(errorLog).arg(__FUNCTION__).arg(__LINE__);
			logger(log);
			return retval;
		}
		
		retval = makeFinalImage();

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

		return true;
	}

	bool addImage(ImageDataRGBPtr image, QString path = "")
	{
		binImages.push_back(std::make_shared<BinImage>(image, imageCount(), path));
		return true;
	}

	//returns true if successfully removed
	bool removeBinImage(int index)
	{
		if (index >= imageCount() || index < 0)
			return false;

		binImages.erase(binImages.begin() + index);

		return true;
	}

	bool removeBinImage(std::vector<int> indices)
	{
		for (auto index : indices)
		{
			if (index >= imageCount() || index < 0)
				return false;
		}
		
		//sort downwards
		std::sort(indices.begin(), indices.end(), [](auto const& lhs, auto const& rhs) 
			{
				return lhs > rhs;
			});

		//remove from large number to small one
		for (auto index : indices)
		{
			binImages.erase(binImages.begin() + index);
		}
		return true;
	}

	void clear()
	{
		//allocate new stack
		binImages = BinImages();
	}
};