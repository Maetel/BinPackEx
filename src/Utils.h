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

	template<typename T>
	void actionPreset(T* action, bool isEnabled, bool isCheckable, bool isChecked)
	{
		if (!action)
			return;
		action->setEnabled(isEnabled);
		action->setCheckable(isCheckable);
		action->setChecked(isChecked);
	}
}