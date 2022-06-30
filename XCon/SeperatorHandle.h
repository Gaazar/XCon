#pragma once
#include "View.h"
namespace FlameUI
{
	enum EDGE
	{
		EDGE_LEFT, EDGE_TOP, EDGE_RIGHT, EDGE_BOTTOM
	};
	class SeperatorHandle :
		public View
	{
		View* left;
		View* top;
		View* right;
		View* bottom;

		bool pressed;
		bool horizontal;
		Point pressPos;

	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
	public:
		SeperatorHandle(View* parent, bool horizontal = false);
		void BindEdge(EDGE edge, View* view);
	};
}
