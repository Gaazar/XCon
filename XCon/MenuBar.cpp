#include "MenuBar.h"
#include "FlameUI.h"

#include <iostream>
using namespace FlameUI;
using namespace D2D1;
void MenuBar::Draw()
{
	auto ctx = BeginDraw(D2D1::ColorF::ColorF(ColorF::Green, 0.0f));
	ID2D1SolidColorBrush* br;
	ID2D1SolidColorBrush* brb;
	float w = rect.width();
	float h = rect.height();
	float x = 0;
	int n = 0;
	if (menu)
	{
		ctx->CreateSolidColorBrush(Theme::Color::Front, &br);
		ctx->CreateSolidColorBrush(ColorF(ColorF::Gray, 0.2f), &brb);
		for (auto i : menu->items)
		{
			if (i.type == MenuItemType_SubMenu)
			{
				if (n == hvrid)
				{
					ctx->FillRectangle({ x,0,x + tws[n] + Theme::LineHeight ,h }, brb);
				}
				ctx->DrawTextW(i.title.c_str(), i.title.length(), root->dTextFormat, { x + Theme::LineHeight / 2,2,w,h }, br, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
				x += tws[n] + Theme::LineHeight;
				n++;
			}
		}
		br->Release();
		brb->Release();

	}
	EndDraw();

}
LRESULT MenuBar::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == FE_LBUTTONDOWN)
	{
		int n = 0;
		float x = 0;
		if (menu)
		{
			for (auto i : menu->items)
			{
				if (i.type == MenuItemType_SubMenu)
				{
					if (n == hvrid)
					{
						auto scp = CalcViewRectOnScreen(this);
						scp.top = scp.bottom;
						scp.left += x;
						PopupMenu((Frame*)root, i.subMenu, nullptr, 0, { (int)scp.left,(int)scp.top });
						break;
					}
					x += tws[n] + Theme::LineHeight;
					n++;
				}
			}

		}

		UpdateView();
	}
	else if (msg == FE_MOUSEMOVE)
	{
		int n = 0;
		float x = 0;
		float mx = (float)GET_X_LPARAM(lParam), my = (float)GET_Y_LPARAM(lParam);
		if (menu)
		{
			for (auto i : menu->items)
			{
				if (i.type == MenuItemType_SubMenu)
				{
					if (PointInRect({ mx,my }, { x,0,x + tws[n] + Theme::LineHeight,Theme::LineHeight + Theme::LinePadding }))
					{
						if (n != hvrid)
						{
							UpdateView();
							hvrid = n;
							if (root->GetKeyboardFocus() == this)
							{
								auto scp = CalcViewRectOnScreen(this);
								scp.top = scp.bottom;
								scp.left += x;
								//std::cout << "AA\n";
								//PopupMenu((Frame*)root, i.subMenu, nullptr, 0, { (int)scp.left,(int)scp.top });
							}
							break;
						}
					}
					x += tws[n] + Theme::LineHeight;
					n++;
				}
			}

		}

	}
	else if (msg == FE_MOUSELEAVE)
	{
		//std::cout << "BB\n";
		hvrid = -1;
		UpdateView();
	}
	return 0;
}
MenuBar::MenuBar(View* parent) :View(parent)
{
	keyable = true;
	size = { 50,Theme::LineHeight + Theme::LinePadding };
	layout.sizeMode = { SIZE_MODE_CONTENT, SIZE_MODE_CONTENT };
}
void MenuBar::SetMenu(Menu* m)
{
	menu = m;
	float ttlw = 0;
	if (menu)
	{
		tws.clear();
		for (auto i : menu->items)
		{
			if (i.type == MenuItemType_SubMenu)
			{
				auto sz = MeasureTextSize(i.title.c_str(), { 99999,99999 }, root->dTextFormat);
				tws.push_back(sz.width);
				sz.width += Theme::LineHeight;
				ttlw += sz.width;

			}
		}
		if (layout.sizeMode.x == SIZE_MODE_CONTENT || layout.sizeMode.y == SIZE_MODE_CONTENT)
		{
			Size({ ttlw,Theme::LineHeight + Theme::LinePadding });
		}
	}
	UpdateView();
}
