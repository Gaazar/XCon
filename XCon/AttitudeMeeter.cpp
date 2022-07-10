#include "AttitudeMeeter.h"
#include "FlameUI.h"
using namespace FlameUI;
using namespace D2D1;
using namespace std;



LRESULT AttitudeMeeter::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
void AttitudeMeeter::Draw()
{

	float ppd = 9;
	auto ctx = BeginDraw(ColorF(0, 0.3843f, 0.5490));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(ColorF(0.5137f, 0.3176f, 0.2118f), &br);
	D2D1_MATRIX_3X2_F rtf;
	D2D1_MATRIX_3X2_F o;
	D2D1_MATRIX_3X2_F mv;
	ctx->GetTransform(&rtf);
	o = rtf * Matrix3x2F::Translation(150, 180) * Matrix3x2F::Scale({ rect.width() / 300.f, rect.height() / 330.f }, { 0,0 });
	//mv = o * Matrix3x2F::Translation(0, ppd * pitch) * Matrix3x2F::Rotation(roll, { -150,0 });
	mv = Matrix3x2F::Translation(0, ppd * pitch) * Matrix3x2F::Rotation(roll, { 0,0 }) * o;
	ctx->SetTransform(mv);
	ctx->FillRectangle({ -9999,0,9999,9999 }, br);
	//ctx->PushAxisAlignedClip({ -150,-150,150,150 }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	br->SetColor(ColorF(0xFFD6B7, 1.f));
	ctx->DrawLine({ -9999,0 }, { 9999,0 }, br, 2);
	for (int i = 1; i <= 9; i++)
	{
		for (int n = 1; n <= 4; n++)
		{
			float y = ppd * 2.5f * (n + (i - 1) * 4);
			float x = 0;
			switch (n)
			{
			case 1:
			case 3:
				x = 18.75f;
				break;
			case 2:
				x = 37.5f;
				break;
			case 4:
				x = 75.f;
				break;
			default:
				break;
			}
			ctx->DrawLine({ -x,y }, { x,y }, br, 2);
			ctx->DrawLine({ -x,-y }, { x,-y }, br, 2);
			if (n == 4)
			{
				auto w = toStringW(i * 10);
				ctx->DrawTextW(w.c_str(), w.length(), fmt, { -x - 40.f,y - 15.f,-x * 1.f,y + 15.f }, br);
				ctx->DrawTextW(w.c_str(), w.length(), fmt, { x,y - 15.f,x + 40.f,y + 15.f }, br);
				ctx->DrawTextW(w.c_str(), w.length(), fmt, { -x - 40.f,-y - 15.f,-x * 1.f,-y + 15.f }, br);
				ctx->DrawTextW(w.c_str(), w.length(), fmt, { x,-y - 15.f,x + 40.f,-y + 15.f }, br);
			}
		}
	}
	ctx->SetTransform(o);
	br->SetColor(ColorF(0x000000, 1.f));
	ctx->FillRectangle({ -3,-3,3,3 }, br);
	ctx->FillRectangle({ -120,-3,-65,3 }, br);
	ctx->FillRectangle({ 65,-3,120,3 }, br);

	br->SetColor(ColorF(0xFFFFFF, 1.f));
	ctx->DrawRectangle({ -3,-3,3,3 }, br);
	ctx->DrawRectangle({ -120,-3,-65,3 }, br);
	ctx->DrawRectangle({ 65,-3,120,3 }, br);
	//ctx->PopAxisAlignedClip();
	br->SetColor(ColorF(0x000000, 1.f));
	ctx->FillRectangle({ -150,-180,150,-150 }, br);
	br->SetColor(ColorF(0xFFFFFF, 1.f));
	D2D1_MATRIX_3X2_F cmv = Matrix3x2F::Translation(yaw * ppd, 0) * o;
	ctx->DrawLine({ -150,-179 }, { 150,-179 }, br, 1);
	auto w = toStringW((int)ry) + L"бу";
	ctx->DrawTextW(w.c_str(), w.length(), fmt, { -40,-180,40,-150 }, br);
	//ctx->SetTransform(cmv);
	//for (int i = 1; i <= 360; i++)
	//{
	//	if (i % 5)
	//	{
	//		//ctx->DrawLine({ i * ppd,-179 }, { 150,-179 }, br, 1);
	//	}
	//	else
	//	{
	//		//ctx->DrawLine({ -150,-179 }, { 150,-179 }, br, 1);

	//	}
	//}

	ctx->SetTransform(rtf);
	br->Release();
	EndDraw();
}

AttitudeMeeter::AttitudeMeeter(View* parent) :View(parent)
{
	gDWFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18, locale, &fmt);
	fmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	fmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	aid = Animate(1000, 0, 0);
}

void AttitudeMeeter::SetYPR(float y, float p, float r)
{
	ry = y;
	if (y < 0)
		ry = 360 + y;
	rp = p;
	rr = -r;
	UpdateView();
}

void AttitudeMeeter::Animation(float p, int, int)
{
	float sf = smoothFast;
	float dy = ry - yaw, dp = rp - pitch, dr = rr - roll;
	if (dp * dp + dr * dr < 5)
	{
		sf = smoothSlow;
	}
	yaw = yaw + dy * sf;
	pitch = pitch + dp * sf;
	roll = roll + dr * sf;

	if (p == 1)
	{
		Animate(1000, 0, 0,aid);
	}
}
