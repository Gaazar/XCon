#include "Panel.h"

using namespace FlameUI;
using namespace D2D1;
Panel::Panel(View* parent) :View(parent)
{
	render.direct = true;
	render.container = true;
	mouseable = false;
	background = D2D1::ColorF::ColorF(0, 0);
	//background = ColorF::ColorF(ColorF::DarkSeaGreen,0.2f);
}

void Panel::Draw()
{
	if (background.a == 0)
	{
		render.direct = false;
	}
	else
	{
		auto ctx = BeginDraw(background);

		EndDraw();
	}
}
