#pragma once
#include "View.h"

namespace FlameUI
{
    class CheckBox :
        public View
    {
    private:
        bool checked;
        bool pressed;
        bool hover;
    protected:
        LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
        void Draw() override;
    public:
        CheckBox(View* parent);
        bool Checked();
        void Checked(bool);
    };
}

