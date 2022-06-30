#include "MenuPainter.h"
#include "FlameUI.h"
#include <iostream>
#include "MenuFrame.h"

using namespace FlameUI;
using namespace D2D1;

void MenuPainter::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(ColorF::Black, 0));
	float offset = 0;
	ID2D1SolidColorBrush* fr, * adp, * dsb;
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Background, 0.3f), &adp);
	ctx->CreateSolidColorBrush(Theme::Color::Front, &fr);
	ctx->CreateSolidColorBrush(ColorF::ColorF(ColorF::Gray), &dsb);
	int idx = 0;
	for (auto i = menu->items.begin(); i != menu->items.end(); i++)
	{
		bool inside = false;
		switch (i->type)
		{
		case MenuItemType_Common:
		case MenuItemType_SubMenu:
			if (mpos.y > offset && mpos.y < offset + Theme::LineHeight + Theme::LinePadding)
			{
				ctx->FillRectangle({ 1.5f,offset + 0.5f,menuSize.width - 1.5f,offset + 0.5f + Theme::LineHeight + Theme::LinePadding }, adp);
				current = idx;
				inside = true;
			}
			if (i->icon)
				ctx->DrawBitmap(i->icon, { 8,8 + offset,24,24 + offset });
			textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			ctx->DrawTextW(i->title.c_str(), i->title.length(), textFormat,
				{ 32,offset,menuSize.width - 32,offset + Theme::LineHeight + Theme::LinePadding },
				i->disabled ? dsb : fr, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
			);
			if (i->type == MenuItemType_Common)
			{
				textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
				ctx->DrawTextW(i->description.c_str(), i->description.length(), textFormat,
					{ 32,offset,menuSize.width - 32,offset + Theme::LineHeight + Theme::LinePadding },
					i->disabled ? dsb : fr, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
				);
			}
			if (i->type == MenuItemType_SubMenu)
			{
				ctx->FillEllipse(Ellipse({ menuSize.width - 16.f,offset + (Theme::LineHeight + Theme::LinePadding) * 0.5f }, 2.0f, 2.0f), fr);
				if (inside)
				{
					if (currentSubmenu != idx)
						PostEvent(this, 0xCCFF, idx, 0);
					currentSubmenuY = offset;
					currentSubmenu = idx;

				}
			}
			offset += Theme::LineHeight + Theme::LinePadding;
			break;
		case MenuItemType_Seperator:
			ctx->DrawLine({ 32.5,offset + Theme::LinePadding / 2 }, { menuSize.width - 1.5f,offset + Theme::LinePadding / 2 }, dsb, 1);
			offset += Theme::LinePadding;
			break;
		default:
			break;
		}
		idx++;
	}
	if (currentSubmenu != current)
	{
		PostEvent(this, 0xccfe, 0, 0);
	}
	fr->Release();
	adp->Release();
	dsb->Release();
	EndDraw();
}
LRESULT MenuPainter::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == FE_MOUSEMOVE)
	{
		MenuFrame* mf = dynamic_cast<MenuFrame*>(root->Parent());
		if (mf)
		{
			mf->ignore = 0;
		}
		mpos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		UpdateView();
	}
	if (msg == FE_MOUSELEAVE)
	{
		mpos = { -5,-5 };
		if (!fLBD)
			current = -1;
	}
	if (msg == FE_LBUTTONDOWN)
	{
		//cout << "LBD; ";
	}
	if (msg == FE_LBUTTONUP)
	{

		if (current >= 0 && menu->items[current].type != MenuItemType_SubMenu && !menu->items[current].disabled)
		{
			fLBD = true;
			parent->SendEvent(WM_KILLFOCUS, -1, 0);
			fLBD = false;
			MenuFrame* mf = dynamic_cast<MenuFrame*>(root->Parent());
			if (mf)
			{
				mf->Close();
			}
		}
		if (current >= 0 && current < menu->items.size() && !menu->items[current].disabled)
		{
			if (callback)
			{
				callback(cbThis, menu, menu->items[current].id);
			}
		}
	}
	if (msg == 0xccff)
	{
		MenuFrame* mf = dynamic_cast<MenuFrame*>(root);
		POINT pt{ -1,-1 };
		if (mf)
		{
			mf->ignore = 1;
			pt = mf->Position();
			pt.x += menuSize.width - 1;
			pt.y += currentSubmenuY;
		}
		submenuFrame = PopupMenu((Frame*)root, menu->items[wParam].subMenu, callback, cbThis, pt);
		if (mf)
		{
			mf->ignore = false;
		}
	}
	if (msg == 0xccfe)
	{
		if (submenuFrame)
		{
			((MenuFrame*)root)->ignore = 1;
			submenuFrame->Close();
			submenuFrame = nullptr;
			currentSubmenu = -1;
			((MenuFrame*)root)->ignore = 0;
		}
	}
	return 0;
}
MenuPainter::MenuPainter(View* parent, Menu* m, MenuCallback cb, void* thiz) :View(parent), menu(m), callback(cb)
{
	cbThis = thiz;
	MenuFrame* mf = dynamic_cast<MenuFrame*>(root);
	if (mf)
	{
		mf->name = m->name.c_str();
	}
	gDWFactory->CreateTextFormat(L"",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Theme::BasicTextSize,
		locale,
		&textFormat
	);
	textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	menuSize = { 0,0 };
	for (auto i = m->items.begin(); i != m->items.end(); i++)
	{
		if (i->type == MenuItemType_Common)
		{
			menuSize.height += Theme::LineHeight + Theme::LinePadding;
			auto rct = MeasureTextSize(i->title.c_str(), SizeF(9999, 9999), root->dTextFormat);
			auto rcd = MeasureTextSize(i->description.c_str(), SizeF(9999, 9999), root->dTextFormat);
			auto width = rct.width + rcd.width + 128;
			if (width > menuSize.width)
			{
				menuSize.width = width;
			}
		}
		else if (i->type == MenuItemType_Seperator)
		{
			menuSize.height += Theme::LinePadding;
		}
		else if (i->type == MenuItemType_SubMenu)
		{
			menuSize.height += Theme::LineHeight + Theme::LinePadding;
			auto rct = MeasureTextSize(i->title.c_str(), SizeF(9999, 9999), root->dTextFormat);
			auto width = rct.width + 128;
			if (width > menuSize.width)
			{
				menuSize.width = width;
			}
		}
	}
	Size(menuSize);
}
