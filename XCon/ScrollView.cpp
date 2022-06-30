#include "ScrollView.h"
#include "FlameUI.h"
#include "Scroller.h"

#include <string>
#include <iostream>

using namespace FlameUI;

// Template of a function listener callback
/*LRESULT cb(View* ds, Message msg, WPARAM wparam, LPARAM lparam)
{
	std::cout << "scrl" << std::endl;
	return 0;
}*/

ScrollView::ScrollView(View* parent) :Panel(parent)
{
	content = new Panel(this);
	content->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);

	scroller.vertical = new Scroller(this);
	scroller.vertical->Coord(COORD_NEGATIVE, COORD_FILL);
	scroller.vertical->Position({ 0, 0 });
	scroller.vertical->Size({ 15, 0 });
	//scroller.vertical->AddEventListener(nullptr, &cb, FE_SCROLL);

	mouseable = true;
}

Panel* ScrollView::Content()
{
	return content;
}

LRESULT ScrollView::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_S_SCROLLING:
	{
		auto v = -scroller.vertical->Offset();
		content->Position({ 0,v });
		break;
	}
	case FE_MOUSEENTER:
	{
		scroller.vertical->Max(content->GetRect().height());
		break;
	}
	case FE_SCROLL:
	{
		//std::cout << "svs" << std::endl;
		return scroller.vertical->SendEvent(msg, wParam, FE_S_L_FROMPARENT);
		break;
	}
	default:
		break;
	}
	return 0;
}
