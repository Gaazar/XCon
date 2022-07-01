#include "Panel.h"

using namespace FlameUI;
using namespace D2D1;
Panel::Panel(View* parent) :View(parent)
{
	mouseable = false;
	background = D2D1::ColorF::ColorF(0, 0);
	//background = ColorF::ColorF(ColorF::DarkSeaGreen,0.2f);
}

void Panel::Draw()
{
	auto ctx = BeginDraw(background);

	EndDraw();

}
