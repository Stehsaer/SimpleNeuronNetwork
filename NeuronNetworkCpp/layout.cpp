#include <iostream>

#include "uni_imgui.h"
#include "UIState.h"

namespace UI
{
	void DrawLayout();

	namespace Layout
	{
		// layout constants
		const int bottomBarHeight = 30;
		
		// element flags
		bool showDemoWindow = false;

		void MainTabBars()
		{
			ImGui::BeginTabItem("Settings##settings_tab", NULL, ImGuiTabItemFlags_None);

			ImGui::Checkbox("Show Demo Window", &showDemoWindow);

			ImGui::EndTabItem();
		}

		void DrawMainPanel()
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight - bottomBarHeight));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			
			ImGui::Begin("MainWindow##main_window", NULL, ImGuiWindowFlags_NoDecoration);

			ImGui::BeginTabBar("MainTab##main_tab", ImGuiTabBarFlags_None);

			MainTabBars();

			ImGui::EndTabBar();

			ImGui::End();

			ImGui::Begin("Bottom Bar##bottom_bar", NULL, ImGuiWindowFlags_NoDecoration);

			ImGui::End();

			ImGui::PopStyleVar(1);
		}
	}

	void DrawLayout()
	{
		Layout::DrawMainPanel();

		if (Layout::showDemoWindow)
			ImGui::ShowDemoWindow();
	}
}