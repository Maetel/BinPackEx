#pragma once

#include "BinImage.h"

//guillotine
#include <algorithm>
#include "GuillotineBinPack.h"

enum BinPackAlgorithm { Guillotine = 0, MaxBinPackAlgorithm };

using BinPackError = int;
#define BP_NO_ERROR 0
#define BP_ERR_NO_IMAGE -1
#define BP_ERR_EXCEED_MAX_IMAGE -2
#define BP_ERR_EXCEED_AVAILABLE_SPACE -3

static std::vector<const char*> BinPackErrorToString
{
	"BINPACK_NO_ERROR",
	"BINPACK_ERR_NO_IMAGE",
	"BINPACK_ERR_EXCEED_MAX_IMAGE",
	"BINPACK_ERR_EXCEED_AVAILABLE_SPACE",
};

class BaseBinPacker
{
public:
	virtual std::vector<rbp::RectSize> binImage2Rects(std::vector<BinImagePtr> const& images) const
	{
		std::vector<rbp::RectSize> retval;

		for (auto ptr : images)
		{
			retval.push_back({ ptr->imagePtr->width(), ptr->imagePtr->height() });
		}

		return retval;
	}
	virtual BinPackError run(int dst_wid, int dst_hi, std::vector<BinImagePtr>& images) = 0;
	virtual ~BaseBinPacker() {}
};

template <int Algorithm = Guillotine>
class BinPacker : public BaseBinPacker
{
};

template <>
class BinPacker<Guillotine> : public BaseBinPacker
{
public:
	using Packer = rbp::GuillotineBinPack;
	std::unique_ptr<Packer> packer = 0;

public:
	BinPackError run(int dst_wid, int dst_hi, std::vector<BinImagePtr>& images) override
	{
		//copy as a workspace
		std::vector<BinImagePtr> reservoir = images;
		
		std::sort(reservoir.begin(), reservoir.end(), [](BinImagePtr const& lhs, BinImagePtr const& rhs)
			{
				return lhs->imagePtr->pixelCount() > rhs->imagePtr->pixelCount();
			});

		packer = std::make_unique<Packer>();
		packer->Init(dst_wid, dst_hi);

		auto Choice = Packer::FreeRectChoiceHeuristic::RectBestAreaFit;
		auto Split = Packer::GuillotineSplitHeuristic::SplitMaximizeArea;

		const bool merge = false;
		std::vector<rbp::Rect> retval;
		for (auto binImage : reservoir)
		{
			const auto img = binImage->imagePtr;
			const auto wid = img->width(), hi = img->height();

			auto result = packer->Insert(wid, hi, merge, Choice, Split);
			if (result.height == 0 || result.width == 0)
			{
				//failed to insert a rectangle
				return BP_ERR_EXCEED_AVAILABLE_SPACE;
			}

			//check if flipped
			const bool flipped = !(result.height == hi && result.width == wid);
			binImage->isFlipped = flipped;

			//update result
			const QPoint startPoint(result.x, result.y);
			const QSize size = !flipped ? QSize(wid, hi) : QSize(hi, wid);
			binImage->result = QRect(startPoint, size);
		}

		//return if successful
		images = reservoir;
		return BP_NO_ERROR;
	}
};