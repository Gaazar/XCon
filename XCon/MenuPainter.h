#pragma once
#include "View.h"
#include "Menu.h"

namespace FlameUI
{

	class MenuFrame;
	class MenuPainter :
		public View
	{
	private:
		Menu* menu;
		MenuCallback callback;
		void* cbThis;
		POINT mpos;
		IDWriteTextFormat* textFormat;
		int current;
		int currentSubmenu;
		int currentSubmenuY;
		MenuFrame* submenuFrame;
		bool fLBD;
	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
	public:
		D2D1_SIZE_F menuSize;
		MenuPainter(View* parent, Menu* m, MenuCallback cb, void* thiz = 0);

	};
}

