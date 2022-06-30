#pragma once
#include "View.h"
#include "TextEditBase.h"
#include "ScrollView.h"
namespace FlameUI
{
    class TextEditor :
        public View
    {
    private:
        ScrollView* scroller;
        TextEditBase* base;
        bool focused;
    protected:
        LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
        void Draw() override;

    public :
        TextEditor(View* parent);
        ~TextEditor();
        void Multiline(bool);
        void Readonly(bool);
        void Password(bool);
        void Tabable(bool);
        void Content(wstring c);
        wstring Content();

    };
    
}