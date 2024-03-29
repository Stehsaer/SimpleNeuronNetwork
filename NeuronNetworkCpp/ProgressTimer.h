#ifndef _PROGRESS_TIMER_H
#define _PROGRESS_TIMER_H

#include <chrono>

struct ProgressTimer
{
	std::chrono::steady_clock::time_point startTimePoint;

	ProgressTimer();
	void Reset();
	long Count();
	long CountAndReset();
};

#endif