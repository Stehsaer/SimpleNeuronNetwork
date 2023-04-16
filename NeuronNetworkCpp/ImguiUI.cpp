#include "ImguiUI.h"

#include <iostream>
#include <thread>

#include "uni_imgui.h"

#include "UIState.h"
#include "UIStyle.h"

// fonts
#include "fonts/RemixIconFontDef.h"
#define ICON_FONT_PATH "fonts/RemixIcon.ttf"
#define HARMONY_FONT_PATH "fonts/Harmony.ttf"

namespace UI
{
	extern void DrawLayout();

	void ImGUILoop()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DrawLayout();
		//ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	const int fontSize = 18;

	void InitIMGUI()
	{
		ImGui::CreateContext();
		ImPlot::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		Style::CustomStyle1();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330 core");

		// Imgui fonts
		io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight; // reduce texture size

		ImVector<ImWchar> range;
		const ImWchar rangeFA[] = {ICON_MIN, ICON_MAX, 0};

		ImFontGlyphRangesBuilder builder;
		builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
		builder.BuildRanges(&range);

		// Add default font
		io.Fonts->AddFontFromFileTTF(HARMONY_FONT_PATH, fontSize, NULL, range.Data); // Load Embedded Font
		
		// Add icon font
		ImFontConfig config;
		config.OversampleH = 1;
		config.OversampleV = 1;
		config.GlyphOffset.y = 3;
		config.MergeMode = true;

		io.Fonts->AddFontFromFileTTF(ICON_FONT_PATH, fontSize + 1, &config, rangeFA);

		io.Fonts->Build();
	}
}