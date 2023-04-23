#pragma once

#include <math.h>

template<typename T, typename T1, typename T2>
inline void Clamp(T& val, T1 min, T2 max)
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