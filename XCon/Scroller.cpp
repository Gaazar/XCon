#include "Scroller.h"
#include "FlameUI.h"

using namespace FlameUI;
using namespace D2D1;


#define LERP(a,b,t) ((a)+((b)-(a))*(t))
#define CLAMP(v,i,x) ( ((v)>=(i)) && ((v)<=(x)) ? v : ((v) < (i)) ? (i) : (x))


void Scroller::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	float tlen = horizontal ? rect.width() : rect.height();
	float hlen = tlen * tlen / max;
	if (hlen < MIN_HANDLE_SIZE) hlen = MIN_HANDLE_SIZE;
	float pcs = handle.current * (tlen - hlen) / (max - tlen);
	if (hlen <= tlen)
	{
		ID2D1SolidColorBrush* br;
		ctx->CreateSolidColorBrush(ColorF::ColorF(ColorF::Gray), &br);

		if (!horizontal)
			ctx->FillRoundedRectangle(
				RoundedRect({ 10 * (1 - animation.prs_hide),pcs,15,pcs + hlen },
					3 * (1 - animation.prs_hide), 3 * (1 - animation.prs_hide))
				, br);
		else
			ctx->FillRoundedRectangle(
				RoundedRect({ pcs, 10 * (1 - animation.prs_hide),pcs + hlen,15 },
					3 * (1 - animation.prs_hide), 3 * (1 - animation.prs_hide))
				, br);
		br->Release();
	}
	EndDraw();
}
void Scroller::Animation(float progress, int p1, int p2)
{
	if (p1 == 0) handle.current = LERP(handle.from, handle.to, 1 - (1 - progress) * (1 - progress));
	if (p1 == 1) animation.prs_hide = 1 - (1 - progress) * (1 - progress);
	if (p1 == 2) animation.prs_hide = (1 - progress) * (1 - progress);
	if (p1 != -1)
		parent->SendEvent(FE_S_SCROLLING, (WPARAM)this, 0);
	UpdateView();
}
LRESULT Scroller::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_SCROLL:
	{
		auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
		float tlen = horizontal ? rect.width() : rect.height();
		handle.to -= delta;
		if (handle.to < 0) handle.to = 0;
		if (handle.to > max - tlen) handle.to = max - tlen;
		handle.from = handle.current;
		if (handle.current <= 0 && delta > 0 || handle.current >= max - tlen && delta < 0)
			return 1;
		else
			Animate(200, 0, 0, animation.scroll);
		return 0;
	}
	case FE_MOUSEENTER:
		//OutputDebugString(L"s me\n");
		Animate(150, 1, 0, animation.hide);
		break;
	case FE_MOUSELEAVE:
		//OutputDebugString(L"s ml\n");
		Animate(150, 2, 0, animation.hide);
		break;
	case FE_LBUTTONDOWN:
	{
		float tlen = horizontal ? rect.width() : rect.height();
		float hlen = tlen * tlen / max;
		if (hlen < MIN_HANDLE_SIZE) hlen = MIN_HANDLE_SIZE;
		float pcs = handle.current * (tlen - hlen) / (max - tlen);
		float pos = horizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
		if (pos < pcs)
		{
			OnEvent(FE_SCROLL, ((short)120) << 16, 0);
		}
		else if (pos > pcs + hlen)
		{
			OnEvent(FE_SCROLL, (-(short)120) << 16, 0);
		}
		else
		{
			delta = pos - pcs;
		}
		break;
	}
	case FE_MOUSEMOVE:
	{
		if (delta >= 0)
		{
			float tlen = horizontal ? rect.width() : rect.height();
			float hlen = tlen * tlen / max;
			if (hlen < MIN_HANDLE_SIZE) hlen = MIN_HANDLE_SIZE;
			float pos = horizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
			float pcs = pos - delta;
			handle.current = handle.to = CLAMP(pcs / (tlen - hlen) * (max - tlen), 0, max - tlen);
			parent->SendEvent(FE_S_SCROLLING, (WPARAM)this, 0);
			UpdateView();
		}
		break;
	}
	case FE_LBUTTONUP:
		delta = -1;
		break;
	}
	return 0;
}
Scroller::Scroller(View* parent) :View(parent)
{
	animation.hide = Animate(0, -1, 0);
	animation.scroll = Animate(0, -1, 0);
	max = 0;
	horizontal = false;
	size = { 15,400 };
	delta = -1;
}
void Scroller::Max(float max)
{
	this->max = max;
	float tlen = horizontal ? rect.width() : rect.height();
	handle.from = handle.current;
	if (handle.current > max - tlen)
	{
		handle.to = max - tlen;
		if(handle.to < 0) handle.to = 0;
		Animate(200, 0, 0, animation.scroll);
	}
	UpdateView();
}
float Scroller::Max()
{
	return this->max;
}
float Scroller::Offset()
{
	return handle.current;
}
