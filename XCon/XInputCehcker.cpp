#include "XInputCehcker.h"

#include "XInput.h"
#include "FlameUI.h"
#pragma comment(lib, "XInput.lib")
#include <iostream>

using namespace std;
using namespace D2D1;
using namespace FlameUI;

LRESULT XInputCehcker::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
void XInputCehcker::Draw()
{
	XINPUT_STATE xs;
	auto r = XInputGetState(index, &xs);

	auto ctx = BeginDraw(D2D1::ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(FlameUI::Theme::Color::Front, &br);

	ctx->DrawRectangle({ 26,0,26 + 128,26 }, br);
	ctx->DrawRectangle({ 26 + 180,0,26 + 180 + 128,26 }, br);
	ctx->DrawEllipse(D2D1::Ellipse({ 90,110 }, 64, 64), br);
	ctx->DrawEllipse(D2D1::Ellipse({ 270,110 }, 64, 64), br);
	for (int i = 0; i < 7; i++)
	{
		ctx->DrawEllipse(D2D1::Ellipse({ 12.f + 48 * i + 24,220 }, 20, 20), br);
		ctx->DrawEllipse(D2D1::Ellipse({ 12.f + 48 * i + 24,300 }, 20, 20), br);
	}



	br->SetColor(ColorF(ColorF::Red));
	ctx->FillRectangle({ 26,0,26 + 128 * xs.Gamepad.bLeftTrigger / 255.f,26 }, br);
	ctx->FillRectangle({ 26 + 180,0,26 + 180 + 128 * xs.Gamepad.bRightTrigger / 255.f,26 }, br);

	ctx->FillEllipse(D2D1::Ellipse({ 90 + xs.Gamepad.sThumbLX / 32768.f * 64,110 - xs.Gamepad.sThumbLY / 32768.f * 64 }, 3, 3), br);
	ctx->FillEllipse(D2D1::Ellipse({ 270 + xs.Gamepad.sThumbRX / 32768.f * 64,110 - xs.Gamepad.sThumbRY / 32768.f * 64 }, 3, 3), br);
	for (int i = 0; i < 7; i++)
	{
		if (i >= 5)
		{
			if (xs.Gamepad.wButtons & (4 << (i * 2)))
				ctx->FillEllipse(D2D1::Ellipse({ 12.f + 48 * i + 24,220 }, 20, 20), br);
			if (xs.Gamepad.wButtons & (4 << (i * 2 + 1)))
				ctx->FillEllipse(D2D1::Ellipse({ 12.f + 48 * i + 24,300 }, 20, 20), br);

		}
		else
		{
			if (xs.Gamepad.wButtons & (1 << (i * 2)))
				ctx->FillEllipse(D2D1::Ellipse({ 12.f + 48 * i + 24,220 }, 20, 20), br);
			if (xs.Gamepad.wButtons & (1 << (i * 2 + 1)))
				ctx->FillEllipse(D2D1::Ellipse({ 12.f + 48 * i + 24,300 }, 20, 20), br);

		}
	}

	if (r != ERROR_SUCCESS)
	{
		br->SetColor(ColorF(ColorF::Gray));
		br->SetOpacity(0.5f);
		ctx->FillRectangle({ 0,0,360,360 }, br);
		br->SetColor(ColorF(ColorF::Red));
		br->SetOpacity(0.5f);
		ctx->DrawText(L"Î´Á¬½Ó", 4, root->dTextFormat, { 360 / 2 - 60 / 2,360 / 2 - 20 / 2,180 + 30,180 + 10 }, br);
	}

	//std::cout << xs.Gamepad.sThumbLX << std::endl;
	EndDraw();
	br->Release();
}
XInputCehcker::XInputCehcker(View* parent) :View(parent)
{
	Size({ 360,360 });
	aid = Animate(1000, 0, 0);
}
void XInputCehcker::Animation(float progress, int p1, int p2)
{
	if (progress == 1)
	{
		Animate(1000, 0, 0, aid);
	}
	UpdateView();
}
