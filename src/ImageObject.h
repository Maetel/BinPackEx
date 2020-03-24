// ImageObject.h

#pragma once

#include <QImage>
#include <QString>
#include "eigen_preset.h"




class ImageObject
{
public:
	virtual int pixelCount() const = 0;
	virtual int length() const = 0; //same as pixelCount
	virtual int pixelSize() const = 0;

	//data size in bytes
	virtual int dataSize() const = 0;
	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual unsigned char* bits() = 0;
	virtual unsigned char const* const bits() const = 0;
	virtual void clear() = 0;
	virtual void resize(int wid, int hi) = 0;
	virtual bool empty() const = 0;
	
	virtual bool isStandardImageType() const = 0;
	virtual bool isGrayType() const = 0;
	virtual bool isRGBType() const = 0;
	virtual bool isRGBAType() const = 0;

public:
	virtual ~ImageObject() {}
};
using ImageObjectPtr = std::shared_ptr<ImageObject>;

template <typename T>
inline T QRgbAllocator(QRgb val)
{
#define IS_T(_type) std::is_same<T, _type>::value

	//check supported types
	static_assert(
		IS_T(unsigned char) ||
		IS_T(VectorRGB) ||
		IS_T(VectorRGBA)
		);

	if constexpr (IS_T(unsigned char))
		return (T)qGray(val);
	
	if constexpr (IS_T(VectorRGB))
		return VectorRGB{ (unsigned char)qRed(val), (unsigned char)qGreen(val), (unsigned char)qBlue(val) };
	
	if constexpr (IS_T(VectorRGBA))
		return VectorRGB{ (unsigned char)qRed(val), (unsigned char)qGreen(val), (unsigned char)qBlue(val), (unsigned char)qAlpha(val) };
}

template <typename T>
class ImageData : public ImageObject
{
public:
	using PixelType = T;

	ImageData() { clear(); }
	ImageData(QString path) { clear();  this->load(path); }
	ImageData(int width, int height, unsigned char* data = nullptr)
	{
		clear();
		_alloc(width, height);
		if (data)
			memcpy(m_data, data, dataSize());
	}
	ImageData(ImageData const& rhs)
	{
		*this = rhs;
	}
	ImageData(ImageData&& rhs)
	{
		clear();
		m_wid = rhs.m_wid;
		m_hi = rhs.m_hi;
		std::swap(m_data, rhs.m_data);
	}
	ImageData& operator=(ImageData const& rhs)
	{
		clear();
		_alloc(rhs.m_wid, rhs.m_hi, rhs.m_data);
		return *this;
	}

	~ImageData() { clear(); }

#pragma region Overrides
	// Overrides

	//type definition field
	template<typename T2>
	constexpr bool pixelTypeIs() const { return std::is_same<T, T2>::value; };
#define IS_GRAY_IMAGE (pixelTypeIs<unsigned char>())
#define IS_RGB_IMAGE (pixelTypeIs<VectorRGB>())
#define IS_RGBA_IMAGE (pixelTypeIs<VectorRGBA>())
#define GRAY_IMAGE_Q_FORM QImage::Format_Grayscale8
#define RGB24_IMAGE_Q_FORM QImage::Format_RGB888
#define RGB32_IMAGE_Q_FORM QImage::Format_RGB32
#define RGBA_IMAGE_Q_FORM QImage::Format_RGB32

	bool isStandardImageType() const override { return isGrayType() || isRGBType() || isRGBAType(); }
	bool isGrayType() const override { return IS_GRAY_IMAGE; }
	bool isRGBType() const override { return IS_RGB_IMAGE; }
	bool isRGBAType() const override { return IS_RGBA_IMAGE; }
	// !type definition field

	unsigned char* bits() override { return reinterpret_cast<unsigned char*>(m_data); }
	unsigned char const* const bits() const override { return reinterpret_cast<unsigned char*>(m_data); }
	int pixelCount() const override { return m_wid * m_hi; }
	int length() const override { return pixelCount(); }
	int pixelSize() const override { return sizeof(T); }
	//data size in bytes	
	int dataSize() const override { return sizeof(T) * m_wid * m_hi; }
	int width() const override { return m_wid; }
	int height() const override { return m_hi; };
	void clear() override { _Delete();  m_wid = default_wid; m_hi = default_hi; };
	void resize(int wid, int hi) override { _alloc(wid, hi); };
	bool empty() const override { return m_data == nullptr; };
	
	// ! Overrides
#pragma endregion
	
#pragma region ImageIO
	// Image IOs
	void fromQImage(QImage const& input)
	{
		auto format = input.format();
		int wid = input.width(), hi = input.height();

		const auto this_form = toQImageFormat();

		if (this_form == QImage::Format_Invalid)
		{
			throw std::logic_error("QImage Format invalid");
		}

		const bool isSameFormat = this_form == format;
		QImage buf = input;
		if (isSameFormat)
		{
			this->_alloc(buf.width(), buf.height(), buf.constBits());
		}
		else
		{
			//buf = buf.convertToFormat(this_form);

			const int wid = buf.width(), hi = buf.height();
			std::vector<T> dataBuf(wid * hi);

			for (int y = 0; y < hi; ++y)
				for (int x = 0; x < wid; ++x)
					dataBuf.at(x + y * wid) = QRgbAllocator<T>(buf.pixel(x, y));

			this->_alloc(wid, hi, dataBuf.data());
		}
	}

	QImage toQImage() const
	{
		QImage retval;

		auto this_form = toQImageFormat();
		if (!m_data || this_form == QImage::Format_Invalid)
			return retval;

		return QImage(bits(), m_wid, m_hi, this_form);
	}

	QImage::Format toQImageFormat() const
	{
		return
			IS_GRAY_IMAGE ? GRAY_IMAGE_Q_FORM :
			IS_RGB_IMAGE ? RGB24_IMAGE_Q_FORM :
			IS_RGBA_IMAGE ? RGB32_IMAGE_Q_FORM :
			QImage::Format_Invalid
			;
	}

	bool load(QString path)
	{
		const auto this_form = toQImageFormat();

		if (this_form == QImage::Format_Invalid)
			return false;

		QImage buf;
		if (!buf.load(path))
			return false;

		fromQImage(buf);

		return true;
	}
	bool save(QString path) const
	{
		return toQImage().save(path);
	}
	// ! Image IOs
#pragma endregion
	
#pragma region Data_Access
	// data accessors
	T& operator()(int idx) { return m_data[idx]; }
	T const& operator()(int idx) const { return m_data[idx]; }
	T& operator()(int x, int y) { return m_data[x + y * m_wid]; }
	T const& operator()(int x, int y) const { return m_data[x + y * m_wid]; }
	T& at(int idx) { return m_data[idx]; }
	T const& at(int idx) const { return m_data[idx]; }
	T& at(int x, int y) { return m_data[x + y * m_wid]; }
	T const& at(int x, int y) const { return m_data[x + y * m_wid]; }

	T* data() { return m_data; }
	T const* const data() const { return m_data; }

	std::vector<T> dataVector() const
	{
		if (!m_data)
			return std::vector<T>();

		std::vector<T> retval(this->pixelCount());
		memcpy(retval.data(), m_data, this->dataSize());
		return retval;
	}
	
	template<typename FuncT>
	void for_each_px(bool parallel, FuncT&& func)
	{
		if (parallel)
			_for_each_px_parallel(std::forward<FuncT>(func));
		else
			_for_each_px_serial(std::forward<FuncT>(func));
	}

	template<typename FuncT>
	void for_each_idx(bool parallel, FuncT&& func)
	{
		if (parallel)
			_for_each_idx_parallel(std::forward<FuncT>(func));
		else
			_for_each_idx_serial(std::forward<FuncT>(func));
	}
	// ! data accessors
#pragma endregion

	void fill(T val)
	{
		std::fill(m_data, m_data + pixelCount(), val);
	}
	void set(T val) { fill(std::forward<T>(val)); }

	//Converter = lambdaFunction(InputType value) {
	// //do something with value...
	// return (OutputType) retval;
	//}
	template <typename OutT, typename Converter>
	ImageData<OutT> convert(Converter&& function) const
	{
		ImageData<OutT> retval(m_wid, m_hi);

		for (int idx = 0; idx < pixelCount(); ++idx)
			retval(idx) = function(m_data[idx]);
		
		return retval;
	}

	template <typename OutT>
	ImageData<OutT> convert() const
	{
		ImageData<OutT> retval(m_wid, m_hi);

		for (int idx = 0; idx < pixelCount(); ++idx)
			retval(idx) = static_cast<OutT>(m_data[idx]);
		
		return retval;
	}

	bool drawSubImage(ImageData<T> const& input, int x, int y, bool rotate90 = false)
	{
		const int wid = this->width(), hi = this->height();
		const int in_wid = rotate90 ? input.height() : input.width();
		const int in_hi = rotate90 ? input.width() : input.height();

		if (x + in_wid >= wid || y + in_hi >= hi)
			return false;

#if 1
		for(int i = 0; i < input.width(); ++i)
			for (int j = 0; j < input.height(); ++j)
			{
				this->at(
					rotate90 ? (x + in_wid - 1 - j) : (x + i),
					rotate90 ? (y + i) : (y + j)
				) = input(i,j);
			}
#else
		if (rotate90)
		{
			input.for_each_px(false, [=](int i, int j, auto const& val)
				{
					this->at(x + in_wid - 1 - j, y + i) = val;
				});
		}
		else
		{
			input.for_each_px(false, [=](int i, int j, auto const& val)
				{
					this->at(x + i, y + j) = val;
				});
		}
#endif
		
		
		return true;
	}

protected:
	void _Delete()
	{
		if (m_data)
			delete m_data;
		m_data = nullptr;
	}
	template<typename T2 = T>
	void _alloc(int wid, int hi, T2 const* data = nullptr)
	{
		clear();
		m_wid = wid;
		m_hi = hi;
		m_data = new T[(size_t)wid * hi];

		if (data)
			memcpy(m_data, reinterpret_cast<unsigned char const* const>(data), dataSize());
	}

	template<typename FuncT>
	void _for_each_px_parallel(FuncT&& func)
	{
		throw std::logic_error("parallel for each px not implemented @" __FUNCTION__);
	}

	template<typename FuncT>
	void _for_each_px_serial(FuncT&& func)
	{
		for (int y = 0; y < m_hi; ++y)
			for (int x = 0; x < m_wid; ++x)
				func(x, y, (*this)(x, y));
	}

	template<typename FuncT>
	void _for_each_idx_parallel(FuncT&& func)
	{
		throw std::logic_error("parallel for each idx not implemented @" __FUNCTION__);
	}

	template<typename FuncT>
	void _for_each_idx_serial(FuncT&& func)
	{
		for (int idx = 0; idx < pixelCount(); ++idx)
			func(idx, *this(idx));
	}

protected:
	const int default_wid = 0, default_hi = 0;
	int m_wid, m_hi;
	T* m_data = 0;
};

#define DECL_PTR(x) using x##Ptr = std::shared_ptr<x>

using ImageData8 = ImageData<unsigned char>;
using GrayImage = ImageData8;
using ImageData16 = ImageData<short>;
using ImageDataFloat = ImageData<float>;
using ImageDataDouble = ImageData<double>;
using ImageDataRGB = ImageData<VectorRGB>;
using RGBImage = ImageData<VectorRGB>;
using ImageDataRGBA = ImageData<VectorRGBA>;
using RGBAImage = ImageData<VectorRGBA>;

DECL_PTR(ImageData8);
DECL_PTR(ImageDataFloat);
DECL_PTR(ImageDataDouble);
DECL_PTR(ImageDataRGB);
DECL_PTR(ImageDataRGBA);

#undef DECL_PTR

#pragma region Inline functions

template<typename T = void>
inline unsigned char RGB2Gray(VectorRGB val)
{
	return
		static_cast<unsigned char>(
			double{ val[0] * 0.2126 } +
			double{ val[1] * 0.7152 } +
			double{ val[2] * 0.0722 }
	);
}

template<typename T = void>
inline ImageData8 RGB2Gray(ImageDataRGB const& image)
{
	return image.convert<unsigned char>([](VectorRGB rgbVal)
		{
			return RGB2Gray(rgbVal);
		});
}

//this method doesn't handle under/overflow
template<typename TOut, typename TIn>
constexpr TOut saturateCast(TIn val)
{
	constexpr auto maxVal = std::numeric_limits<TOut>::max();
	constexpr auto minVal = std::numeric_limits<TOut>::min();
	return
		(val > maxVal) ? maxVal :
		(val < minVal) ? minVal :
		val;
}

#pragma endregion