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

	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
	public:
		Dropdown(View* parent);
		bool Editable();
		void Editable(bool);
		void Candidates(Menu*);

	};
}

