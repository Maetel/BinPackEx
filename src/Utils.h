#pragma once

namespace util
{
#define KorStr(x) QString::fromLocal8Bit(x)
	
	template<typename T>
	void HandyDelete(T*& ptr)
	{
		if (ptr)
			delete ptr;
		ptr = nullptr;
	}

	template<typename T>
	void actionPreset(T* action, bool isEnabled, bool isCheckable, bool isChecked)
	{
		if (!action)
			return;
		action->setEnabled(isEnabled);
		action->setCheckable(isCheckable);
		action->setChecked(isChecked);
	}

	template<typename FloatType = double>
	constexpr FloatType Inch2cm(FloatType inch)
	{
		return inch * (FloatType)2.54;
	}

	template<typename FloatType = double>
	constexpr FloatType Inch2mm(FloatType inch)
	{
		return inch * (FloatType)25.4;
	}

	template<typename FloatType = double>
	constexpr FloatType cm2Inch(FloatType cm)
	{
		return cm * (FloatType)0.39370079;
	}

	template<typename FloatType = double>
	constexpr FloatType px2mm(int pixel, int DPI)
	{
		return (FloatType)Inch2mm(((FloatType)pixel / (FloatType)DPI));
	}
}