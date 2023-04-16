#include "UIState.h"

namespace UI
{
	// Window
	bool windowVsync = true;
	int windowWidth = 1280, windowHeight = 720;
	GLFWwindow* window;

	bool terminateWindow = false;

	UIWindowState uiState = UIWindowState::Unloaded;

	Components::NotifyQueue notifyQueue;
	Components::NotifyWindow notifyWindow(notifyQueue);

	void AddWarning(std::string msg)
	{
		notifyQueue.Queue(Components::Notification(msg, "Error", 0));
	}
}