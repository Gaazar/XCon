#include "FlameUI.h"
#include "Button.h"

using namespace FlameUI;

Button::Button(View* parent) :View(parent)
{
	size = { 80,30 };
	layout.anchor = { 0.5f,0.5f };
}

LRESULT Button::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_MOUSEENTER:
	case FE_MOUSELEAVE:
	{
		hover = msg == FE_MOUSEENTER;
		UpdateView();
		break;
	}
	case FE_LBUTTONDOWN:
		pressd = true;
		scale = { 0.98f,0.98f };
		UpdateTransform();
		UpdateView();
		break;
	case FE_LBUTTONUP:
		pressd = false;
		UpdateTransform();
		UpdateView();
		Animate(150, 0, 0);
		break;
	default:
		break;
	}
	return 0;
}
using namespace D2D1;
void Button::Draw()
{
	auto ctx = BeginDraw(AdaptColor(Theme::Color::Background,0.35f,0));
	if (!br_bdr)
	{
		ctx->CreateSolidColorBrush(ColorF::ColorF(ColorF::White), &br_bdr);
		ctx->CreateSolidColorBrush(ColorF::ColorF(ColorF::Gray, 0.5f), &br_ovl);
	}
	if (pressd)
		ctx->FillRectangle({ 0,0,rect.width(),rect.height() }, br_ovl);
	if (hover && !pressd)
		ctx->DrawRectangle({ 1.5f,1.5f,rect.width() - 1.5f,rect.height() - 1.5f }, br_ovl, Theme::BorderWidth);
	EndDraw();
}

void Button::Animation(float progress, int p1, int p2)
{
	//progress = Easings::QuintOut(progress);
	scale = { 0.98f + 0.02f * progress,0.98f + 0.02f * progress };
	UpdateTransform();
	UpdateView();
}
