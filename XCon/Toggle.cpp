#include "Toggle.h"
#include "FlameUI.h"

using namespace FlameUI;
using namespace D2D1;

LRESULT Toggle::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_MOUSEENTER:
	{
		hover = true;
		UpdateView();
		break;
	}
	case FE_MOUSELEAVE:
	{
		hover = false;
		UpdateView();
		break;
	}
	case FE_LBUTTONDOWN:
	{
		pressed = true;
		UpdateView();
		break;
	}case FE_LBUTTONUP:
	{
		pressed = false;
		Checked(!checked);
		break;
	}
	}
	return 0;
}

void Toggle::Draw()
{
	float fac = Theme::LineHeight / 24.f;
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* front, * adapted, * theme, * atheme;
	ctx->CreateSolidColorBrush(Theme::Color::Front, &front);
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Front), &adapted);
	ctx->CreateSolidColorBrush(Theme::Color::Theme, &theme);
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Theme), &atheme);
	if (!front || !adapted || !theme || !atheme)
	{
		EndDraw();
		return;
	}

	D2D1_ROUNDED_RECT shapeBack = RoundedRect({ 0.5f, rect.height() / 2 - 11 * fac,0.5f + 47 * fac,rect.height() / 2 + 11 * fac }, 11 * fac, 11 * fac);
	if (checked)
	{
		if (pressed)
			ctx->FillRoundedRectangle(shapeBack, adapted);
		else
			if (hover)
				ctx->FillRoundedRectangle(shapeBack, atheme);

			else
				ctx->FillRoundedRectangle(shapeBack, theme);
		ctx->FillEllipse(Ellipse({ fac * (46.5f - 11 - 24 * Easings::QuintIn(1 - animation.progress)),rect.height() / 2 }, 6 * fac, 6 * fac), front);
	}
	else
	{
		if (pressed)
			ctx->FillRoundedRectangle(shapeBack, adapted);
		else
			if (hover)
				ctx->DrawRoundedRectangle(shapeBack, front, Theme::BorderWidth);
			else
				ctx->DrawRoundedRectangle(shapeBack, adapted, Theme::BorderWidth);
		ctx->FillEllipse(Ellipse({ fac * (46.5f - 11 - 24 * Easings::QuintOut(animation.progress)),rect.height() / 2 }, 6 * fac, 6 * fac), front);
	}
	front->Release();
	adapted->Release();
	theme->Release();
	atheme->Release();
	EndDraw();
}
Toggle::Toggle(View* parent) :View(parent)
{
	checked = pressed = hover = false;
	layout.box.padding.left = 50 * Theme::LineHeight / 24.f;
	size = { 50,25 };
	animation.id = Animate(0, -1, 0);
	animation.progress = 1;

}
bool Toggle::Checked()
{
	return checked;
}
void Toggle::Checked(bool c)
{
	checked = c;
	Animate(300, 0, 0, animation.id);
	UpdateView();
	SendEvent(FE_CHANGE, 0, 0);
}

void Toggle::Animation(float progress, int p1, int p2)
{
	if (p1 != -1)
		animation.progress = progress;
	UpdateView();
}
