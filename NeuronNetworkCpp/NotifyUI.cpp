#include "NotifyUI.h"
#include "uni_imgui.h"

#include "UIState.h"

#include "fonts/RemixIconFontDef.h"

#include <iostream>

namespace UI
{
	namespace Components
	{
		void NotifyQueue::Queue(Notification notify)
		{
			queue.push_back(notify);
			newItemAdded = true;
		}

		void NotifyQueue::Update()
		{
			TimePoint now = SteadyClock::now();
			
			for (auto iter = queue.begin(); iter != queue.end();)
			{
				auto& notify = *iter;
				if (notify.close || ((now - notify.start).count() / 1000000 > notify.duration && notify.duration > 0))
				{
					iter = queue.erase(iter);
				}
				else
				{
					iter++;
				}
			}
		}

		void NotifyQueue::Clear()
		{
			queue.clear();
		}

		bool NotifyQueue::NewItemAdded()
		{
			if (newItemAdded)
			{
				newItemAdded = false;
				return true;
			}
			else return false;
		}
		
		void NotifyWindow::Render()
		{
			queue.Update();

			if (queue.queue.size() > 0)
			{
				ImGui::Spacing();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::Button(ICON_DELETE_BIN_FILL " Clear all"))
				{
					queue.queue.clear();
				}
				ImGui::NewLine();

				for (int i = queue.queue.size() - 1; i >= 0; i--)
				{
					auto& item = queue.queue[i];

					ImGui::Unindent();
					ImGui::SeparatorText(item.title.c_str());
					ImGui::Spacing();
					ImGui::Indent();

					const int wordLength = 60;
					ImGui::TextWrapped(item.msg.length() > wordLength ? std::format("{}...", item.msg.substr(0, wordLength)).c_str() : item.msg.c_str());

					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
					{
						if (ImGui::BeginTooltip())
						{
							ImGui::Text(item.msg.c_str());
							ImGui::EndTooltip();
						}
					}

					ImGui::Spacing();
					if (ImGui::Button(item.GenId("Dismiss").c_str(), ImVec2(70, 30)))
					{
						item.callback();
						item.close = true;
					}

					ImGui::NewLine();
				}
			}
			else
			{
				ImGui::NewLine();
				ImGui::Indent();  ImGui::Text("No Notification");
			}

		}
	}
}