#include "LinearPlacer.h"

using namespace FlameUI;

LinearPlacer::LinearPlacer(DIRECTION dir, float space)
{
	direction = dir;
	this->space = space;
}


void LinearPlacer::Layout(vector<View*>& children)
{
	float offset = 0;
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		View& c = **i;
		if (c.Alpha() < 1)
		{
			if (direction == DIRECTION_VERTICAL)
			{
				c.Position({ c.Position().x,offset });
				offset += c.GetRect().height() + space + c.Margin().top + c.Margin().bottom;
			}
			if (direction == DIRECTION_HORIZONTAL)
			{
				c.Position({ offset ,c.Position().y });
				offset += c.GetRect().width() + space + c.Margin().left + c.Margin().right;;
			}
		}
	}
}
