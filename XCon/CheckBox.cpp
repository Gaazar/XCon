#include "CheckBox.h"
#include "FlameUI.h"

using namespace FlameUI;
using namespace D2D1;

LRESULT CheckBox::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
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

void CheckBox::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* front, * adapted, * theme;
	ctx->CreateSolidColorBrush(Theme::Color::Front, &front);
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Front), &adapted);
	ctx->CreateSolidColorBrush(Theme::Color::Theme, &theme);
	float fac = Theme::LineHeight / 24.f;
	if (!front || !adapted || !theme)
	{
		EndDraw();
		return;
	}

	D2D1_RECT_F shapeBack{ 0.5f,rect.height() / 2 - 10 * fac,20.5f * fac,rect.height() / 2 + 10 * fac };
	D2D1_RECT_F shapeFront{ Theme::BorderWidth,rect.height() / 2 - 10 * fac + Theme::BorderWidth,20 * fac,rect.height() / 2 + 10 * fac - Theme::BorderWidth };
	if (pressed)
	{
		ctx->FillRectangle(shapeBack, adapted);
	}
	else
	{
		if (hover)
		{
			if (checked)
				ctx->FillRectangle(shapeBack, theme);
			ctx->DrawRectangle(shapeFront, front, Theme::BorderWidth);
		}
		else
		{
			if (checked)
				ctx->FillRectangle(shapeBack, theme);
			else
				ctx->DrawRectangle(shapeFront, adapted, Theme::BorderWidth);
		}
	}
	if (checked)
	{
		//ctx->DrawLine({ 3 * fac,11 }, { 8 * fac,16 }, front, 1.5 * fac);
		//ctx->DrawLine({ 8 * fac,16 }, { 18 * fac,6 }, front, 1.5 * fac);
		ctx->DrawLine({ 3 * fac,rect.height() / 2 }, { 8 * fac,rect.height() / 2 + 5 * fac }, front, 1.5 * fac);
		ctx->DrawLine({ 8 * fac,rect.height() / 2 + 5 * fac }, { 18 * fac,rect.height() / 2 - 5 * fac }, front, 1.5 * fac);

	}
	front->Release();
	adapted->Release();
	theme->Release();
	EndDraw();
}
CheckBox::CheckBox(View* parent) :View(parent)
{
	checked = pressed = hover = false;
	layout.box.padding.left = 25* Theme::LineHeight / 24.f;
	size = { 22,22 };

}
bool CheckBox::Checked()
{
	return checked;
}
void CheckBox::Checked(bool c)
{
	checked = c;
	UpdateView();
	SendEvent(FE_CHANGE, 0, 0);
}