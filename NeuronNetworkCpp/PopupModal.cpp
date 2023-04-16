#include "PopupModal.h"
#include <format>
#include "UIState.h"

namespace UI
{
	namespace Components
	{
		//==Component Class====

		void PopupModal::Open()
		{
			popupState = State::Opening;
		}

		void PopupModal::Close()
		{
			popupState = State::Closing;
		}

		void PopupModal::SetReturnPtr(void* ptr)
		{
			returnPtr = ptr;
		}

		bool PopupModal::Opened()
		{
			return (popupState == State::Opening || popupState == State::Opened);
		}

		bool PopupModal::Closed()
		{
			return (popupState == State::Closing || popupState == State::Closed);
		}

		std::string PopupModal::GenId()
		{
			return std::format("{}##{:#x}", title, self);
		}

		const ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		void PopupModal::Render()
		{
			if (popupState == State::Opening)
			{
				ImGui::OpenPopup(GenId().c_str());
				openFlag = true;
				popupState = State::Opened;
			}
			if (!openFlag && hasCloseButton)
			{
				popupState = State::Closed;
			}

			// center window
			ImGui::SetNextWindowPos(ImVec2(windowWidth / 2, windowHeight / 2), 0, ImVec2(0.5, 0.5));
			ImGui::SetNextWindowSize(ImVec2(width, 0));
			if (ImGui::BeginPopupModal(GenId().c_str(), hasCloseButton ? &openFlag : NULL, popupFlags))
			{
				func(this, returnPtr);

				if (popupState == State::Closing)
				{
					ImGui::CloseCurrentPopup();
					popupState = State::Closed;
					openFlag = false;
				}
				ImGui::EndPopup();
			}
		}

		void PopupDisplayer::Push(PopupModal* modal)
		{
			displayList.push_back(modal);
		}

		void PopupDisplayer::Push(std::initializer_list<PopupModal*> list)
		{
			for (auto item : list)
			{
				displayList.push_back(item);
			}
		}

		PopupDisplayer::PopupDisplayer(std::initializer_list<PopupModal*> list)
		{
			Push(list);
		}

		void PopupDisplayer::Display()
		{
			for (auto item : displayList)
			{
				try
				{
					item->Render();
				}
				catch (std::exception e)
				{
					AddWarning(std::format("Unhandled Exception: {0}", e.what()));
				}
			}
		}

	}
}