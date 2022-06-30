#pragma once
#include "View.h"
namespace FlameUI
{
	class Toggle :
		public View
	{
	private:
		bool checked;
		bool pressed;
		bool hover;
		struct
		{
			animation_id id;
			float progress;
		} animation;
		float mdOffset;
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
		void Animation(float progress, int p1, int p2) override;
	public:
		Toggle(View* parent);
		bool Checked();
		void Checked(bool);
	};
}

