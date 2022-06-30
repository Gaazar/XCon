#include "Label.h"
#include "FlameUI.h"
#include <iostream>
using namespace FlameUI;
using namespace D2D1;
using namespace std;
#pragma comment(lib, "dwrite.lib")

Label::Label(View* parent, wstring content) :View(parent)
{
	layout.sizeMode = { SIZE_MODE_CONTENT ,SIZE_MODE_CONTENT };
	gDWFactory->CreateTextFormat(L"",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Theme::BasicTextSize,
		locale,
		&format
	);
	mouseable = false;
	//keyable = true;
	Content(content);
	//localTransform = Matrix3x2F::Translation({ 20,20 });
	UpdateTransform();
	frontColor = Theme::Color::Front;
	//render.direct = true;
}
Label::Label(View* parent) :Label(parent, L"")
{
}

void Label::Content(wstring content)
{
	EnterCriticalSection(&gThreadAccess);
	this->content = content;
	Refresh();
	UpdateView();
	LeaveCriticalSection(&gThreadAccess);
}
void Label::Refresh()
{
	if (layout.sizeMode.x == SIZE_MODE_CONTENT || layout.sizeMode.y == SIZE_MODE_CONTENT)
	{
		Size(MeasureTextSize(content.c_str(), { 9999,9999 }, format));
	}
}
wstring Label::Content()
{
	return this->content;
}
void Label::Draw()
{
	auto ctx = BeginDraw(D2D1::ColorF::ColorF(ColorF::Green, 0.0f));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(frontColor, &br);
	ctx->DrawTextW(content.c_str(), content.length(), format, { 0,0,size.width,size.height }, br, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
	br->Release();
	EndDraw();
}
LRESULT Label::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	//if (msg == FE_MOUSEENTER)  OutputDebugStringA("Label ME\n");
	//if (msg == FE_MOUSELEAVE)  OutputDebugStringA("Label ML\n");
	//if (msg == FE_LBUTTONCLICK) OutputDebugStringA("Label Clicked\n");
	//if (msg == FE_MOUSEMOVE) std::cout << (((int)lParam) >> 16) << "," << ((int)lParam & 0x0000FFFF) << std::endl;
	return 0;
}
void Label::Font(IDWriteTextFormat* format)
{
	if (this->format)
		this->format->Release();
	format->AddRef();
	this->format = format;
	Refresh();
	UpdateView();
}
