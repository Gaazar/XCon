#pragma once
#include "Frame.h"
namespace FlameUI
{
    class MenuFrame :
        public Frame
    {
        void OnKillFocus(View* v, Message m, WPARAM w, LPARAM l);
    public:
        int ignore;
        MenuFrame(Frame* parent,POINT rPos,SIZE size);
    };
}
