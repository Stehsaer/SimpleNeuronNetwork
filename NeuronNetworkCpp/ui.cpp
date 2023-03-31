#include "ui.h"

#include <iostream>
#include <thread>

#include "uni_imgui.h"

#include "ImguiUI.h"

namespace UI
{
	void InitGL()
	{
		// glfw
		if (!glfwInit())
		{
			throw std::exception("Unable to initialize GLFW");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(windowWidth, windowHeight, "Neuron Network", NULL, NULL);

		if (!window)
		{
			glfwTerminate();
			throw std::exception("Unable to start UI window");
		}

		glewInit();

		glfwSwapInterval(windowVsync);
		glfwSetWindowSizeLimits(window, windowWidth, windowHeight, windowWidth, windowHeight);
		glfwMakeContextCurrent(window);
	}

	void InitUI()
	{
		InitGL();
		InitIMGUI();
	}

	void MainLoop()
	{
		try
		{
			uiState = UIWindowState::Starting;
			InitUI();

			// loop
			uiState = UIWindowState::Loaded;

			while (!glfwWindowShouldClose(window))
			{
				// clear color
				glClearColor(.0f, .0f, .0f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT);

				ImGUILoop();

				// swap buffers
				glfwSwapBuffers(window);
				glfwPollEvents();
			}

			uiState = UIWindowState::Close;
			glfwTerminate();

			return;
		}
		catch (std::exception e)
		{
			uiState = UIWindowState::Error;
			std::cout << "[ERROR] Error occured in <UI>. Msg: " << e.what() << std::endl;
		}
	}
}