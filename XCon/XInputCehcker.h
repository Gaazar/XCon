#pragma once
#include "View.h"
namespace FlameUI
{
    class XInputCehcker :
        public View
    {
        void Animation(float progress, int p1, int p2) override;
        animation_id aid;

    protected:
        LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
        void Draw() override;

    public:
        int index = 0;
        XInputCehcker(View* parent);
    };
}
