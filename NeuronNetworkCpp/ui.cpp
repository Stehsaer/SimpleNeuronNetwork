#include "ui.h"

#include <iostream>
#include <thread>

#include "uni_imgui.h"

#include "ImguiUI.h"
#include "Tasks.h"

namespace UI
{
	void Draw();

	// callback for resizing the window
	void WindowResizeCallback(GLFWwindow* window, int width, int height)
	{
		windowWidth = width;
		windowHeight = height;
	}

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

		glfwSetWindowSizeLimits(window, 800, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
		glfwMakeContextCurrent(window);

		glfwSetWindowSizeCallback(window, WindowResizeCallback);
	}

	void InitUI()
	{
		InitGL();
		InitIMGUI();

		namespace inst = Tasks::Instances;
		//inst::ReadFileList(inst::datasetFiles, inst::datasetPath, { ".images", ".labels" });
		inst::ReadDirList(inst::datasetDirs, inst::datasetPath);
		inst::ReadFileList(inst::networkFileList, inst::networkPath, { ".network" });
	}

	void Draw()
	{
		// clear color
		glClearColor(.0f, .0f, .0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGUILoop();

		// swap buffers
		glfwSwapInterval(windowVsync);
		glfwSwapBuffers(window);
		glfwPollEvents();
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
				Draw();
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