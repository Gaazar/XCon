#pragma once
#include "View.h"
namespace FlameUI
{
    class RadioButton :
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
        RadioButton(View* parent);
        bool Checked();
        void Checked(bool);
    };
}

