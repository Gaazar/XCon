#pragma once
#include "basictypes.h"
#include "View.h"

namespace FlameUI
{
	class IDockProvider
	{
	public:
		virtual void Preview(View* content,Point mousePos) = 0;
		virtual void Place(View* content,Point mousePos) = 0;
	};
}