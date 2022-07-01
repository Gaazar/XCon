#pragma once
#include "View.h"
namespace FlameUI
{
    class Chart :
        public View
    {
    protected:
        LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
        void Draw() override;

    };
}

