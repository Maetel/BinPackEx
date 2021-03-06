#pragma once

#include <QString>
#include <stack>
#include "BinImage.h"
#include "ImagePathParser.h"
#include "BinPacker.h"

// * header only class
// contains followings :
//		BinImages
//		BInPacking Algorithm
//		Destination size (resultSize)
//		Last state of BinImages
class BinImageManager
{
public:
	ImagePathParser m_parser;
	std::unique_ptr<BaseBinPacker> binPacker = 0;

	using BinImages = std::vector<BinImagePtr>;
	BinImages binImages;
	QSize resultSize{ -1,-1 };

	BinImages lastState;
public:

	void storeCurState()
	{
		lastState = binImages;
	}

	void restoreLastState()
	{
		binImages = lastState;
	}

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
	BinImagePtr imageAt(int index)
	{
		if (index >= imageCount())
			return BinImagePtr();
		for (auto& ptr : binImages)
			if (ptr->imageIndex == index)
				return ptr;

		return BinImagePtr() = 0;
	}
	BinImagePtr const imageAt(int index) const
	{
		if (index >= imageCount())
			return BinImagePtr();
		for (auto ptr : binImages)
			if (ptr->imageIndex == index)
				return ptr;

		return 0;
	}

	// Copied values, karlsuns extracted explicitly
	// these karlsuns will not be updated even if BinImages are updated
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
			throw;
			break;
		}
	}

	ImageDataRGBPtr makeFinalImage(VectorRGB background = VectorRGB::White()) const
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
		
		//update image itself if rotated
		for (auto& ptr : binImages)
			ptr->imagePtr = ptr->eval();

		retval = makeFinalImage();

		return retval;
	}

	//bin pack logger muted
	ImageDataRGBPtr binPack()
	{
		return binPack([](QString msg) {/* mute */});
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

	void updateIndices()
	{
		for (int idx = 0; idx < imageCount(); ++idx)
			binImages.at(idx)->imageIndex = idx;
	}

	//returns true if successfully removed
	bool removeBinImage(int index)
	{
		if (index >= imageCount() || index < 0)
			return false;

		int startPos = 0;
		for (auto it = binImages.begin(); it != binImages.end(); ++it)
		{
			if (**it == *imageAt(index))
			{
				binImages.erase(binImages.begin() + startPos);
				updateIndices();
				return true;
			}
			startPos++;
		}

		return false;
	}

	bool removeBinImage(std::vector<int> indices)
	{
		if (indices.empty())
			return false;

		//sort downwards
		std::sort(indices.begin(), indices.end(), [](auto const& lhs, auto const& rhs)
			{
				return lhs > rhs;
			});

		//return false if contains an index larger than image count
		if (*(indices.begin()) >= imageCount())
			return false;

		//remove from large number to small one
		for (auto index : indices)
			if (const bool suc = removeBinImage(index); !suc)
				return false;

		return true;
	}

	void clear()
	{
		binImages = BinImages();
	}
};