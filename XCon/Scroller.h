#pragma once
#include "View.h"

#define MIN_HANDLE_SIZE (30)
namespace FlameUI
{
#define FE_S_SCROLLING 0x20A0
	class Scroller :
		public View
	{
	private:
		bool horizontal = false;
		float max;
		struct
		{
			animation_id hide;
			animation_id scroll;
			float prs_hide;
			float prs_scroll;
		} animation;
		struct {
			float current;
			float to;
			float from;
			float delta;
		} handle;
		float delta;
	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Animation(float progress, int p1, int p2) override;
	public:
		Scroller(View* parent);
		void Max(float max);
		float Max();
		float Offset();

	};
}

