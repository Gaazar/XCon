#pragma once
#include "View.h"
#include "TextEditBase.h"
#include "Label.h"
#include "Menu.h"

namespace FlameUI
{
	class Dropdown :
		public View
	{
		TextEditBase* te;
		Label* lbTrigger;
		Menu* menu;

	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
	public:
		Dropdown(View* parent);
		bool Editable();
		void Editable(bool);
		void Candidates(Menu*);
		Menu* Candidates();
		std::wstring Content();
		void Content(std::wstring);
		void Cantidate(UINT64 id);
		UINT64 Candidate();

	};
}

