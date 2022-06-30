#pragma once
#include "View.h"
#include "ImGui.h"
namespace FlameUI
{
	class ImGuiCanvas :
		public View
	{

		ImGui::Context context;
		virtual void Intereact();
		volatile bool lockk;
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
		void IntereactPrepare();
	public:
		ImGuiCanvas(View* parent);

	};
}