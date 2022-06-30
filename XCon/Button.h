#pragma once
#include "View.h"
namespace FlameUI
{
	class Button :
		public View
	{
    private:
        float animation;
        float pressd;
        float hover;
        ID2D1SolidColorBrush* br_bdr = nullptr;
        ID2D1SolidColorBrush* br_ovl = nullptr;
    protected:
        void Draw() override;
        LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
        void Animation(float progress, int p1, int p2) override;
    public:
        Button(View* parent);

	};

}