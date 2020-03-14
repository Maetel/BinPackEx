
#include <algorithm>
#include "GuillotineBinPack.h"
#include "opencv2/highgui.hpp"
#include "main.h"

struct Image
{
	int wid, hi;
	std::shared_ptr<cv::Mat> img = 0;
	
	int offset;
	rbp::Rect result;
	rbp::Rect thomson;

	Image(std::string path)
	{
		auto _img = std::make_shared<cv::Mat>(cv::imread(path));
		wid = _img->cols;
		hi = _img->rows;
		img = _img;
	}

	void updateThomson(int offset)
	{
		this->offset = offset;
		thomson = rbp::Rect{ result.x + offset, result.y + offset, result.width - 2 * offset, result.height - 2 * offset };
	}
};

void loadImages(std::vector<Image>& images, int start, int end)
{
	std::vector<Image> retval;
	for (int idx = start; idx < end; ++idx)
	{
		std::string pre("C:/work/cpp/BinPackEx/resources/images/img (");
		std::string post(").jpg");
		std::string path = pre + std::to_string(idx) + post;
		retval.push_back(Image(path));
	}
	images = retval;
}

std::vector<rbp::RectSize> images2rects(std::vector<Image> const& images)
{
	std::vector<rbp::RectSize> retval;
	for (auto const& img : images)
	{
		retval.push_back(rbp::RectSize{img.wid, img.hi});
	}
	return retval;
}

std::vector<rbp::Rect> getResults(int wid, int hi, std::vector<rbp::RectSize> const& rects)
{
	using Packer = rbp::GuillotineBinPack;
	Packer packer;
	packer.Init(wid, hi);
	auto Choice = Packer::FreeRectChoiceHeuristic::RectBestAreaFit;
	auto Split = Packer::GuillotineSplitHeuristic::SplitMaximizeArea;

	bool merge = !true;
	auto _rects = rects;

#if 0
	packer.Insert(_rects, merge, Choice, Split);
	//return packer.GetUsedRectangles();
	return packer.GetFreeRectangles();
#else
	//preserving order
	std::vector<rbp::Rect> retval;
	for (auto rect : rects)
	{
		retval.push_back(packer.Insert(rect.width, rect.height, merge, Choice, Split));
	}
	return retval;
#endif
}

void partialCopy(cv::Mat& large, Image const& small, rbp::Rect rect)
{
	bool dir_org = (small.wid == rect.width) && (small.hi == rect.height);
	bool dir_flip = (small.hi == rect.width) && (small.wid == rect.height);

	if (!(dir_org || dir_flip))
		return;

	if (dir_org)
	{
		int wid = small.wid, hi = small.hi;
		int startX = rect.x, startY = rect.y;
		for (int x = 0; x < wid; ++x)
			for (int y = 0; y < hi; ++y)
				large.at<cv::Vec3b>(y+startY, x+startX) = small.img->at<cv::Vec3b>(y, x);
	}
	else
	{
		//flipped
		int wid = small.hi, hi = small.wid;
		int startX = rect.x, startY = rect.y;
		for (int x = 0; x < wid; ++x)
			for (int y = 0; y < hi; ++y)
				large.at<cv::Vec3b>(y + startY, x + startX) = small.img->at<cv::Vec3b>(x, y);
	}
}

cv::Mat makeFinalImg(int wid, int hi, std::vector<Image> const& images, std::vector<rbp::Rect> const& rects)
{
	cv::Mat retval;
	if (images.size() != rects.size())
		return retval;
	retval = cv::Mat(hi, wid, CV_8UC3);
	std::fill(retval.data, retval.data + hi * wid * 3, (uchar)255);

	int cnt = rects.size();
	for(int idx = 0; idx < cnt; ++idx)
	for (auto rect : rects)
	{
		partialCopy(retval, images.at(idx), rects.at(idx));
	}

	return retval;
}

//void updateImages()

int main(void)
{
	std::vector<Image> images;
	loadImages(images, 1, 8);
	std::sort(images.begin(), images.end(), [](const Image& lhs, const Image& rhs)
		{
			const auto pixelCount = [](cv::Mat const& mat)->int { return mat.cols * mat.rows; };
			return pixelCount(*lhs.img) > pixelCount(*rhs.img);
		});

	constexpr int dstWid = 1400, dstHi = 1000;
	auto initialRects = images2rects(images);
	auto results = getResults(dstWid, dstHi, initialRects);
	
	//int thomsonOffset = 10;
	//updateImages(images, results, thomsonOffset);

	auto resultImg = makeFinalImg(dstWid, dstHi, images, results);
	cv::imshow("Result", resultImg);

	cv::waitKey(0);

	return 0;
}