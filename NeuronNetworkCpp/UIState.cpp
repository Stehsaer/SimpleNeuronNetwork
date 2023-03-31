#include "UIState.h"

namespace UI
{
	// Window
	bool windowVsync = true;
	int windowWidth = 1280, windowHeight = 720;
	GLFWwindow* window;

	bool terminateWindow = false;

	UIWindowState uiState = UIWindowState::Unloaded;
}