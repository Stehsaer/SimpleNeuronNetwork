#pragma once

#ifndef _NOTIFY_UI_H_
#define _NOTIFY_UI_H_

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include "uni_imgui.h"

namespace UI
{
	namespace Components
	{
		namespace chrono = std::chrono;
		typedef chrono::steady_clock SteadyClock;
		typedef chrono::time_point<SteadyClock> TimePoint;
		typedef chrono::nanoseconds NanoSec;

		struct Notification
		{
		public:

			struct Color
			{
				float r, g, b;
				Color(float r, float g, float b): r(r), g(g), b(b){}
				Color(int r, int g, int b): r(r / 255.0), g(g / 255.0), b(b/ 255.0){}
			};

			std::string msg;
			std::string title;

			unsigned long duration; // in milliseconds; 0 for never expire
			bool close;

			TimePoint start;

			std::function<void(void)> callback;

			Notification(std::string messageText, std::string titleText, unsigned long durationMs, std::function<void(void)> callback = [] {}) :
				msg(messageText),
				title(titleText),
				duration(durationMs),
				close(false),
				callback(callback),
				start(SteadyClock::now()) {}

			inline std::string GenId(std::string prefix)
			{
				return prefix + std::format("##{:#x}", reinterpret_cast<uintptr_t>(this)); // use reinterpret_cast to avoid warnings
			}
		};

		class NotifyQueue
		{
		public:
			std::vector<Notification> queue;
			bool newItemAdded;

			NotifyQueue() :newItemAdded(false) {};

			void Update();
			void Queue(Notification notify);
			void Clear();
			bool NewItemAdded();
		};

		class NotifyWindow
		{
		public:
			NotifyQueue& queue;
			bool fadeOut;

			NotifyWindow(NotifyQueue& queue, bool enableFadeOut = true) :queue(queue), fadeOut(enableFadeOut) {}

			void Render();
		};
	}
}

#endif