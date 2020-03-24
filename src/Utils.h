#pragma once

namespace util
{
	template<typename T>
	void HandyDelete(T*& ptr)
	{
		if (ptr)
			delete ptr;
		ptr = nullptr;
	}
}