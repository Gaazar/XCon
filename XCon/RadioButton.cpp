#include "RadioButton.h"
#include "FlameUI.h"

using namespace FlameUI;
using namespace D2D1;

LRESULT RadioButton::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
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
	}
	case FE_LBUTTONUP:
	{
		pressed = false;
		Checked(true);
		break;
	}
	case FE_RADIO:
	{
		if (wParam != (WPARAM)this)
			Checked(false);
	}
	}
	return 0;
}

void RadioButton::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* front, * adapted, * theme;
	ctx->CreateSolidColorBrush(Theme::Color::Front, &front);
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Front), &adapted);
	ctx->CreateSolidColorBrush(Theme::Color::Theme, &theme);
	if (!front || !adapted || !theme)
	{
		EndDraw();
		return;
	}

	float fac = Theme::LineHeight / 24.f;
	D2D1_ELLIPSE shapeBack = Ellipse({ 10 * fac,rect.height() / 2 }, 9 * fac, 9 * fac);
	D2D1_ELLIPSE shapeFront = Ellipse({ 10 * fac,rect.height() / 2 }, 4.5 * fac, 4.5 * fac);
	if (hover)
	{
		if (checked)
			if (pressed)
			{
				ctx->FillEllipse(shapeFront, adapted);
				ctx->DrawEllipse(shapeBack, adapted, Theme::BorderWidth);
			}
			else
			{
				ctx->FillEllipse(shapeFront, front);
				ctx->DrawEllipse(shapeBack, theme, Theme::BorderWidth);
			}
		else
			if (pressed)
			{
				ctx->DrawEllipse(shapeBack, adapted, Theme::BorderWidth);
			}
			else
			{
				ctx->DrawEllipse(shapeBack, front, Theme::BorderWidth);
			}
	}
	else
	{
		if (checked)
		{
			ctx->FillEllipse(shapeFront, adapted);
			ctx->DrawEllipse(shapeBack, theme, Theme::BorderWidth);
		}
		else
			ctx->DrawEllipse(shapeBack, adapted, Theme::BorderWidth);

	}

	front->Release();
	adapted->Release();
	theme->Release();
	EndDraw();
}
RadioButton::RadioButton(View* parent) :View(parent)
{
	checked = pressed = hover = false;
	layout.box.padding.left = 25 * Theme::LineHeight / 24.f;
	size = { 22,22 };

}
bool RadioButton::Checked()
{
	return checked;
}
void RadioButton::Checked(bool c)
{
	checked = c;
	if (checked)
	{
		parent->SendEvent(FE_RADIO_BROADCAST, (WPARAM)this, 0);
	}
	UpdateView();
}