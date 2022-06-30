#pragma once
#include "View.h"
#include "Label.h"
#include <vector>
namespace FlameUI
{
#define SBE_OPTIONCHANGED 0x2FA0
#define SBE_INTERNAL_OPTIONCHANGED 0x2FB0
	class SectionBar :
		public View
	{
		struct Option
		{	
			Label* display;
			HANDLE listener;
			View* content;
		};
		vector<Option> items;
		float anim;
		animation_id aid;
		int curnt = 0;
		int prev = 0;
		IDWriteTextFormat* deffmt;
		LRESULT ItemCallback(View* view, Message msg, WPARAM wparam, LPARAM lparam);
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
		void Animation(float progress, int p1, int p2) override;

	public:
		SectionBar(View* parent);
		void AddOption (const wchar_t* dspl ,View* content);
		int Select(int index);
	};
}
