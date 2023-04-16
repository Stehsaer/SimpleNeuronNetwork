#pragma once

#include <math.h>

template<typename T>
inline void Clamp(T& val, T min, T max)
{
	if (val < min)
	{
		val = min; return;
	}
	if (val > max)
	{
		val = max; return;
	}
}