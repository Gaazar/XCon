#pragma once
#include "View.h"
namespace FlameUI
{
	enum DIRECTION
	{
		DIRECTION_VERTICAL,
		DIRECTION_HORIZONTAL,
		DIRECTION_VERTICVL_REVERSE,
		DIRECTION_HORIZONTAL_REVERSE
	};
	class LinearPlacer :
		public Placer
	{
		DIRECTION direction;
		float space;
	public:
		LinearPlacer(DIRECTION dir = DIRECTION_VERTICAL, float space = 0);
		void Layout(vector<View*>& children) override;
	};
}

