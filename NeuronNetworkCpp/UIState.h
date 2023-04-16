#pragma once

#ifndef _UI_STATE_H_
#define _UI_STATE_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include "NotifyUI.h"
#include "PopupModal.h"

namespace UI
{
	enum class UIWindowState
	{
		Unloaded,
		Starting,
		Loaded,
		Error,
		Close
	};

	// Window
	extern bool windowVsync;
	extern int windowWidth, windowHeight;
	extern GLFWwindow* window;

	extern bool terminateWindow;

	extern UIWindowState uiState;

	extern Components::NotifyQueue notifyQueue;
	extern Components::NotifyWindow notifyWindow;

	void AddWarning(std::string msg);
}

#endif