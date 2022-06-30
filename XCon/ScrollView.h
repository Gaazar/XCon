#pragma once
#include "View.h"
#include "Scroller.h"
#include "Panel.h"
namespace FlameUI
{
	class ScrollView :public Panel
	{
	private:
		struct
		{
			Scroller* vertical;
			Scroller* horizontal;

		} scroller;
		Panel* content;
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
	public:
		ScrollView(View* parent);
		Panel* Content();
	};
}