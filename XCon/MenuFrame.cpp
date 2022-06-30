#include "MenuFrame.h"
#include <iostream>
using namespace FlameUI;


MenuFrame::MenuFrame(Frame* parent, POINT rPos, SIZE size) :Frame(parent, size, rPos, WS_EX_TOPMOST | WS_EX_NOACTIVATE)
{
	sizable = false;
	maximal = false;
	minimal = false;
	dragble = false;
	close = false;
	Title(L"MenuFrame");
	titled = false;
	ignore = 0;
	//cout << rPos.x << "," << rPos.y << endl;
	AddEventListener(this, &MenuFrame::OnKillFocus, WM_KILLFOCUS);
}

void MenuFrame::OnKillFocus(View* v, Message m, WPARAM w, LPARAM l)
{
	if (!ignore)
	{
		Close();
		MenuFrame* mf = dynamic_cast<MenuFrame*>(root->Parent());
		if (mf)
		{
			mf->SendEvent(WM_KILLFOCUS, w--, 0);

		}
	}

}


