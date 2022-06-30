#include "SectionBar.h"
#include "FlameUI.h"
#include "LinearPlacer.h"
using namespace FlameUI;
using namespace D2D1;

#define LERP(a,b,t) ((a)+((b)-(a))*(t))

LRESULT SectionBar::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case SBE_INTERNAL_OPTIONCHANGED:
	{
		if (wParam != curnt)
		{
			prev = curnt;
			curnt = wParam;
			Animate(500, 0, 0, aid);
			SendEvent(SBE_OPTIONCHANGED, (WPARAM)items[curnt].content, (LPARAM)items[prev].content);
		}
		break;
	}
	case FE_SIZED:
	{
		UpdateView();
		break;
	}
	}

	return 0;
}
void SectionBar::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(Theme::Color::Theme, &br);
	if (br)
	{
		float prR = anim * 2, prL = 0;
		if (prR > 1)
			prR = 1;
		prL = (anim - 0.45f) / 0.55f;
		if (prL < 0)
			prL = 0;
		if (items.size() > 0)
		{
			if (curnt > prev)
			{
				prR = Easings::QuintIn(prR);
				prL = Easings::QuintOut(prL);
			}
			else
			{
				float t = prR;
				prR = Easings::QuintOut(prL);
				prL = Easings::QuintIn(t);

			}
			float Rb, Lb, Re, Le;
			Rb = items[prev].display->GetRect().right;
			Lb = items[prev].display->GetRect().left;
			Re = items[curnt].display->GetRect().right;
			Le = items[curnt].display->GetRect().left;
			ctx->FillRectangle({ LERP(Lb,Le,prL) - 1,rect.height() - 3 ,LERP(Rb,Re,prR), rect.height() }, br);
		}

		br->Release();
	}
	EndDraw();
}
SectionBar::SectionBar(View* parent) :View(parent)
{
	Padding({ 0,5,0,5 });
	Layouter(new LinearPlacer(DIRECTION_HORIZONTAL));
	SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	aid = Animate(0, -1, 0);
	wchar_t locale[85];
	GetUserDefaultLocaleName(locale, sizeof(locale));
	gDWFactory->CreateTextFormat(L"",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		18,
		locale,
		&deffmt
	);
}

void SectionBar::AddOption(const wchar_t* dspl, View* content)
{
	Label* l = new Label(this, dspl);
	l->Font(deffmt);
	l->Coord(COORD_POSTIVE, COORD_CENTER);
	l->Margin(Rect(15, 0, 0, 0), true);
	l->mouseable = true;
	PostEvent(this, FE_CHILDSIZED, 0, 0);
	auto hSn = l->AddEventListener(this, &SectionBar::ItemCallback, FE_LBUTTONUP);


	items.push_back({ l,hSn,content });
}

void SectionBar::Animation(float progress, int p1, int p2)
{
	anim = progress;
	UpdateView();
}
int SectionBar::Select(int index)
{
	int ret = -1;
	if (index >= 0 && index < items.size())
	{
		ret = curnt;
		SendEvent(SBE_INTERNAL_OPTIONCHANGED, index, (LPARAM)items[index].content);
	}
	return ret;
}

LRESULT SectionBar::ItemCallback(View* view, Message msg, WPARAM wparam, LPARAM lparam)
{
	for (auto i = items.begin(); i != items.end(); i++)
	{
		if (i->display == view)
		{
			SendEvent(SBE_INTERNAL_OPTIONCHANGED, i - items.begin(), (LPARAM)i->content);
			break;
		}
	}
	return 0;
}


