#pragma once
#include "View.h"
namespace FlameUI
{
	class AttitudeMeeter :
		public View
	{
	private:
		animation_id aid;
		IDWriteTextFormat* fmt;
		float yaw;
		float pitch;
		float roll;

		float ry, rp, rr;

	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
		void Animation(float, int, int) override;

	public:
		float smoothFast = 0.2f;
		float smoothSlow = 0.05f;
		AttitudeMeeter(View* parent);
		void SetYPR(float y, float p, float r);
	};
}
