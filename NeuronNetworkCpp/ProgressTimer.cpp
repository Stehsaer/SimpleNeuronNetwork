#include "ProgressTimer.h"

ProgressTimer::ProgressTimer()
{
	Reset();
}

void ProgressTimer::Reset()
{
	startTimePoint = std::chrono::steady_clock::now();
}

long ProgressTimer::Count()
{
	return (long)((std::chrono::steady_clock::now() - startTimePoint).count());
}

long ProgressTimer::CountAndReset()
{
	long temp = Count();
	Reset();
	return temp;
}