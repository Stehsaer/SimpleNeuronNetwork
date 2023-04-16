#pragma once

#ifndef _POPUP_MODAL_H_
#define _POPUP_MODAL_H_

#include <string>
#include <functional>
#include "uni_imgui.h"

namespace UI
{
	namespace Components
	{
		class PopupModal
		{
		private:
			enum class State
			{
				Closed,
				Opening,
				Opened,
				Closing
			} popupState;

			void* returnPtr;
			std::function<void(PopupModal*, void*)> func;
			std::string title;
			bool hasCloseButton;
			bool openFlag;
			int width;

			uintptr_t self;

		public:
			
			PopupModal(std::string title, std::function<void(PopupModal*, void*)> func, void* returnPtr, int width = 0, bool hasCloseButton = false): 
				title(title), 
				func(func),
				returnPtr(returnPtr),
				hasCloseButton(hasCloseButton), 
				popupState(State::Closed), 
				self(reinterpret_cast<uintptr_t>(this)), 
				width(width),
				openFlag(false) {};

			std::string GenId();

			void SetReturnPtr(void* ptr);
			void SetTitle(std::string title);

			void Open();
			void Close();

			bool Opened();
			bool Closed();

			void Render();
		};

		class PopupDisplayer
		{
		public:
			std::vector<PopupModal*> displayList;

			PopupDisplayer() {};
			PopupDisplayer(std::initializer_list<PopupModal*> list);

			void Push(PopupModal* modal);
			void Push(std::initializer_list<PopupModal*> list);

			void Pop(PopupModal* modal);

			void Display();
		};
	}
}

#endif