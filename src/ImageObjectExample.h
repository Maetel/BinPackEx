// ImageObjectExample.h

#include "ImageObject.h"
#include <QDebug>

template <typename T = void>
class _HereIs
{
public:
	_HereIs(QString welcomeMsg) : msg(welcomeMsg) { qDebug() << '\n' << "==========" << welcomeMsg << "==========" ; }
	~_HereIs() { qDebug() << "==========" << "Finished [" << msg <<"]" << "==========" << '\n'; }

	QString msg;
};
#define HereIs(x) _HereIs __hereisinstance(QString(#x))

template<typename T = void>
class ImageObjectExample
{
public:
	ImageObjectExample() { run(); }

	void run()
	{
		QString dirpath = "C:/work/cpp/BinPackEx/resources/images/";
		QString someRGB = QString("%1%2").arg(dirpath).arg("img (1).jpg");
		QString toGray = QString("%1%2").arg(dirpath).arg("img (1)-gray.jpg");
		QString grayAsRGB = QString("%1%2").arg(dirpath).arg("img (1)-gray-rgb.jpg");

		ImageData8Ptr grayImage = std::make_shared<ImageData8>();
		grayImage->load(someRGB); //if RGB image loaded into a gray image format, it automatically converts to gray type
		grayImage->save(toGray);

		ImageDataRGBPtr rgbImage = std::make_shared<ImageDataRGB>(toGray); //load gray-fied image as RGB type
		rgbImage->save(grayAsRGB);


		{
			HereIs(show basic infos);

			qDebug() << QString("Is image loaded : %1, is image rgb type : %2, is image gray type : %3")
				.arg(!rgbImage->empty())
				.arg(rgbImage->isRGBType())
				.arg(rgbImage->isGrayType());
			qDebug() << QString("Width = %1 height = %2, pixelCount = %3")
				.arg(rgbImage->width())
				.arg(rgbImage->height())
				.arg(rgbImage->pixelCount());
			qDebug() << QString("Pixel size in bytes : %1, Total data size in bytes : %2")
				.arg(rgbImage->pixelSize())
				.arg(rgbImage->dataSize());
		}

		{
			HereIs(using base class methods);

			std::vector<ImageObjectPtr> images
			{
				grayImage,
				rgbImage
			};

			for (auto const& img : images)
			{
				qDebug() << "Is this image gray type ? : " << img->isGrayType();
			}
		}
		
		{
			HereIs(converting images to other types);

			ImageDataRGB someRGBImage(someRGB);
			someRGBImage.save("beforeCasted.bmp");
			ImageData8 dst = someRGBImage.convert<unsigned char>([](VectorRGB rgbVal)
				{
					return RGB2Gray(rgbVal);
				});
			dst.save("castedToGray.bmp");

			ImageData16 dst16 = dst.convert<short>();
		}

		
		{
			HereIs(manipulating each pixel);

			ImageData8 grayImage(someRGB);
			ImageData8 brighterGrayImage = grayImage;

			bool computeParallel = false; // parallel is not implemented yet
			brighterGrayImage.for_each_px(computeParallel, [](int x, int y, auto& px)
				{
					px = saturateCast<unsigned char, int>(px + 50); //make brighter
				});

			grayImage.save("someGrayImg.bmp");
			brighterGrayImage.save("someBrighterGrayImg.bmp");
		}
	}
};