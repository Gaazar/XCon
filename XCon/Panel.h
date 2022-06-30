#pragma once
#include "View.h"
#include "FlameUI.h"

namespace FlameUI
{
	class Panel :
		public View
	{
	public:
		Color background;
		Panel(View* parent);
		void Draw() override;

	};
}
