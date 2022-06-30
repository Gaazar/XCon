#pragma once
#include "View.h"
#include <string>
#include "Menu.h"

using namespace std;
namespace FlameUI
{
#define Cut() (Copy();Backspace();)
	class TextEditBase :
		public View
	{
	private:
		wstring content;
		struct
		{
			bool multiline;
			bool readonly;
			bool password;
			bool tabable;
		} flags;
		struct
		{
			bool pressed;
			bool ctrl;
			bool shift;
			bool focusing;
			bool compositing;
		}states;
		struct
		{
			UINT32 from;
			UINT32 to;
			ID2D1PathGeometry* geometry;
			UINT32 complen;

		}selection;
		struct
		{
			animation_id id;
			float opacity;
		}animation;
		IDWriteTextLayout* textLayout;
		IDWriteTextFormat* textFormat;
		Menu* texteditMenu;
		void MakeGeom();
		void Refresh();
		const wchar_t* Copy();
		void Paste(wstring txt);
		void Paste();
		void Backspace();
		void MakeHightlight();
		void MenuCallback(Menu* m, int id);
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
		void Animation(float progress, int p1, int p2) override;
	public:
		TextEditBase(View* parent);
		void Content(wstring c);
		wstring Content();
		void Multiline(bool);
		void Readonly(bool);
		void Password(bool);
		void Tabable(bool);

	};
}