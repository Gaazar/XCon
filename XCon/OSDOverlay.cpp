#include "OSDOverlay.h"
#include "FlameUI.h"

using namespace FlameUI;
using namespace D2D1;

float smoother(float& v, float t, float  s, float  f, float throt)
{
	float sm = s;
	if (abs(v - t) > throt) sm = f;
	v = v + (t - v) * sm;
	return v;
}

LRESULT OSDOverlay::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
void OSDOverlay::Draw()
{
	auto ctx = BeginDraw(ColorF(0, 0, 0, 0));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(ColorF(0, 1, 0), &br);
	D2D1_MATRIX_3X2_F rtf;
	D2D1_MATRIX_3X2_F o;
	D2D1_MATRIX_3X2_F mv;
	ctx->GetTransform(&rtf);

	float w = rect.width();
	float h = rect.height();
	float horzsz = 0.8f;
	float ppd = 18;
	float pps = 6;
	float ppa = 0.8f;
	float ppc = 10.f;


	//Pitch&Roll
	{
		o = Matrix3x2F::Translation({ 0,ppd * pitch_d }) * Matrix3x2F::Rotation(roll_d, { 0,0 }) * Matrix3x2F::Translation({ w / 2,h / 2 }) * rtf;
		ctx->SetTransform(o);
		ctx->DrawLine({ -w * horzsz * 0.5f,0 }, { w * horzsz * 0.5f,0 }, br, 3);
		ctx->SetTransform(rtf);
		ctx->PushAxisAlignedClip({ 80,0.15f * h,w - 120, h }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		ctx->SetTransform(o);
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
					auto w = toStringW(-i * 10);
					ctx->DrawTextW(w.c_str(), w.length(), fmt, { -x - 40.f,y - 15.f,-x * 1.f,y + 15.f }, br);
					ctx->DrawTextW(w.c_str(), w.length(), fmt, { x,y - 15.f,x + 40.f,y + 15.f }, br);
					w = toStringW(i * 10);
					ctx->DrawTextW(w.c_str(), w.length(), fmt, { -x - 40.f,-y - 15.f,-x * 1.f,-y + 15.f }, br);
					ctx->DrawTextW(w.c_str(), w.length(), fmt, { x,-y - 15.f,x + 40.f,-y + 15.f }, br);
				}
			}
		}
		ctx->PopAxisAlignedClip();
	}
	//RollMeeter
	{
		ctx->SetTransform(rtf);
		ctx->PushAxisAlignedClip({ 0.25f * w,0,0.75f * w,0.25f * h }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		ctx->DrawTextW(L"▲", 1, fmt, { 0.5f * w - 10,70,0.5f * w + 10,90 }, br);
		o = Matrix3x2F::Rotation(roll_d, { 0,0 }) * Matrix3x2F::Translation({ 0.5f * w,h * 0.5f }) * rtf;
		ctx->SetTransform(o);
		ctx->DrawEllipse(Ellipse({ 0,0 }, 0.4f * h, 0.4f * h), br, 2);
		ctx->DrawTextW(L"0", 1, fmt, { -30,-0.4f * h - 30 ,30,-0.4f * h - 5 }, br);
		for (int i = 0; i < 12; i++)
		{
			int deg = 15 + i * 15;
			auto w = to_wstring(deg);
			ctx->SetTransform(Matrix3x2F::Rotation(deg, { 0,0 }) * o);
			ctx->DrawTextW(w.c_str(), w.length(), fmt, { -30,-0.4f * h - 30 ,30,-0.4f * h - 5 }, br);
			ctx->SetTransform(Matrix3x2F::Rotation(-deg, { 0,0 }) * o);
			ctx->DrawTextW(w.c_str(), w.length(), fmt, { -30,-0.4f * h - 30 ,30,-0.4f * h - 5 }, br);

		}
		ctx->PopAxisAlignedClip();
	}
	//Yaw
	{
		ctx->SetTransform(Matrix3x2F::Translation({ w / 2,60 }) * rtf);
		auto w = to_wstring((int)yaw_d) + L"°";
		ctx->DrawTextW(w.c_str(), w.length(), fmt, { -30,-60,30,-20 }, br);
	}
	//AirSpeed
	{
		ctx->SetTransform(rtf);
		ctx->PushAxisAlignedClip({ 0,0.2f * h,80,0.8f * h }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		o = Matrix3x2F::Translation(0, 0.5f * h) * rtf;
		ctx->SetTransform(o);
		ctx->DrawTextW(L"⏴", 1, fmt, { 70,-10,80,10 }, br);
		for (int i = -5; i < 5; i++)
		{
			int asp = aspeed_d / 10;
			asp = asp * 10 + 10 * i;
			float y = (asp - aspeed_d) * pps;
			auto w = to_wstring(asp);
			ctx->DrawTextW(w.c_str(), w.length(), fmt, { 0,-y - 10,50,-y + 10 }, br);
			ctx->FillRectangle({ 50,-y - 2,70,-y + 2 }, br);
			ctx->FillRectangle({ 60,-y + pps * 5 - 1,70,-y + pps * 5 + 1 }, br);
		}
		ctx->DrawRectangle({ 3,-10,50,10 }, br, 1);
		br->SetOpacity(0.7f);
		br->SetColor(ColorF(ColorF::Black));
		ctx->FillRectangle({ 3,-10,50,10 }, br);
		br->SetOpacity(1);
		br->SetColor(ColorF(1, 1, 0));
		auto w = to_wstring((int)(aspeed_d));
		ctx->DrawTextW(w.c_str(), w.length(), fmt, { 3,-10,50,10 }, br);
		ctx->PopAxisAlignedClip();
		br->SetColor(ColorF(0, 1, 0));
	}
	//Alt&Climb
	{
		ctx->SetTransform(rtf);
		ctx->PushAxisAlignedClip({ w - 100,0.2f * h,w,0.8f * h }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		o = Matrix3x2F::Translation(w - 80, 0.5f * h) * rtf;
		ctx->SetTransform(o);
		ctx->DrawTextW(L"⏴", 1, fmt, { 30,-10,40,10 }, br);
		for (int i = -5; i < 5; i++)
		{
			int asp = alt_d / 100;
			asp = asp * 100 + 100 * i;
			float y = (asp - alt_d) * ppa;
			auto w = to_wstring(asp);
			ctx->DrawTextW(w.c_str(), w.length(), fmt, { 30,-y - 10,80,-y + 10 }, br);
			ctx->FillRectangle({ 10,-y - 2,30,-y + 2 }, br);
			ctx->FillRectangle({ 10,-y + ppa * 50 - 1,20,-y + ppa * 50 + 1 }, br);
		}
		ctx->DrawRectangle({ 40,-10,80,10 }, br, 1);
		br->SetOpacity(0.7f);
		br->SetColor(ColorF(ColorF::Black));
		ctx->FillRectangle({ 40,-10,80,10 }, br);
		br->SetOpacity(1);
		br->SetColor(ColorF(1, 1, 0));
		auto w = to_wstring((int)(alt_d));
		ctx->DrawTextW(w.c_str(), w.length(), fmt, { 33,-10,80,10 }, br);
		br->SetColor(ColorF(0, 1, 0));
		ctx->FillRectangle({ -10,-ppc * climb_d,0,0 }, br);
		ctx->PopAxisAlignedClip();
	}
	//Info
	{
		ctx->SetTransform(rtf);
		wchar_t infotxt[128];
		auto l = wsprintf(infotxt, L"经度: %s°\n纬度: %s°\n地速: %s m/s", to_wstring(lat_d).c_str(), to_wstring(lon_d).c_str(), to_wstring(gspeed_d).c_str());
		ctx->DrawTextW(infotxt, l, fmt, { w - 250,h - 120,w,h }, br);

	}
	ctx->SetTransform(Matrix3x2F::Translation(w / 2, h / 2) * rtf);
	br->SetColor(ColorF(0x00FF00, 1.f));
	ctx->FillRectangle({ -3,-3,3,3 }, br);
	ctx->FillRectangle({ -120,-3,-65,3 }, br);
	ctx->FillRectangle({ 65,-3,120,3 }, br);

	br->SetColor(ColorF(0xFFFF00, 1.f));
	ctx->DrawRectangle({ -3,-3,3,3 }, br);
	ctx->DrawRectangle({ -120,-3,-65,3 }, br);
	ctx->DrawRectangle({ 65,-3,120,3 }, br);


	br->Release();
	EndDraw();

}
OSDOverlay::OSDOverlay(View* parent) :View(parent)
{
	aid = Animate(1000, 0, 0);
	gDWFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18, FlameUI::locale, &fmt);
	fmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	fmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);


}
void OSDOverlay::Animation(float p, int, int)
{
	lon_d = rawdata.lon;
	lat_d = rawdata.lat;
	smoother(aspeed_d, rawdata.aspeed, smooth.meter_s, smooth.meter_f, 1);
	smoother(gspeed_d, rawdata.gspeed, smooth.meter_s, smooth.meter_f, 1);
	smoother(climb_d, rawdata.climb, smooth.meter_s, smooth.meter_f, 0.2f);
	smoother(alt_d, rawdata.alt, smooth.meter_s, smooth.meter_f, 2.0f);
	smoother(yaw_d, rawdata.yaw, smooth.degs_s, smooth.degs_f, 2.0f);
	smoother(pitch_d, rawdata.pitch, smooth.degs_s, smooth.degs_f, 2.0f);
	smoother(roll_d, rawdata.roll, smooth.degs_s, smooth.degs_f, 2.0f);
	if (p == 1)
	{
		Animate(1000, 0, 0, aid);
	}
	//roll_d += ((float)rand() / RAND_MAX) * 5 - 2.5f;
	//pitch_d += ((float)rand() / RAND_MAX) * 2 - 1.f;
	//yaw_d += ((float)rand() / RAND_MAX) * 5 - 2.5f;
	//aspeed_d += 0.016f;
	//alt_d += 0.2f;
	//climb_d = 10;
	UpdateView();
}

void OSDOverlay::SetYPR(float y, float p, float r)
{
	rawdata.yaw = y;
	rawdata.pitch = p;
	rawdata.roll = r;
}
void OSDOverlay::SetGPS(float lat, float lon)
{
	rawdata.lat = lat;
	rawdata.lon = lon;
}
void OSDOverlay::SetSpeeds(float as, float gs)
{
	rawdata.aspeed = as;
	rawdata.gspeed = gs;
}
void OSDOverlay::SetAltc(float alt, float climb)
{
	rawdata.alt = alt;
	rawdata.climb = climb;
}
