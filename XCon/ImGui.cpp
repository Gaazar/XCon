#include "ImGui.h"
#include "FlameUI.h"
#include <algorithm>
using namespace FlameUI;
using namespace ImGui;

Context* ImGui::currentCotext = nullptr;
IDWriteTextFormat* ImGui::defaultFont = nullptr;

ID ImGui::HashString(String s)
{
	hash<String> hsh;
	return hsh(s) + currentCotext->seed++;
}
ID ImGui::HashStringSL(String s)
{
	hash<String> hsh;
	return hsh(s);
}
void MoveDrawpos(FlameUI::Size sz, bool space = true)
{
	auto& ctx = *currentCotext;
	ctx.prevY = ctx.drawPos.y;
	ctx.prevX = ctx.drawPos.x + sz.width + FlameUI::Theme::LinePadding;
	ctx.drawPos.y += sz.height + (space ? FlameUI::Theme::LinePadding : 0);
	ctx.drawPos.x = ctx.currentClipRect.left;

}
bool BhvrButton(ID id, FlameUI::Rect rc, bool& press, bool& held)
{
	auto& ctx = *currentCotext;
	bool ret = false;
	held = false;
	press = false;
	if (PointInRect(ctx.cursorPos, rc))
	{
		held = true;
		press = ctx.lButtonDown;
		if (ctx.event == FE_LBUTTONDOWN)
		{
			ctx.focus = id;
		}
		if (ctx.event == FE_LBUTTONUP)
		{
			if (ctx.focus == id)
				ret = true;
		}
	}

	return ret;
}

void ImGui::SameLine(float x, float spacing)
{
	auto& ctx = *currentCotext;

	ctx.drawPos.x = ctx.prevX;
	ctx.drawPos.y = ctx.prevY;
}


void ImGui::Seperator()
{
	auto& ctx = *currentCotext;

	ctx.renderContext.DrawLine(
		{ ctx.drawPos.x,ctx.drawPos.y + 0.5f * Theme::LinePadding },
		{ ctx.currentClipRect.right,ctx.drawPos.y + 0.5f * Theme::LinePadding },
		ColorF(ColorF::Gray, 0.5f),
		Theme::BorderWidth);

	MoveDrawpos({ ctx.currentClipRect.width(),Theme::BorderWidth });
	//ctx.prevY = ctx.drawPos.y;
	//ctx.prevX = 0;
	//ctx.drawPos.y += Theme::BorderWidth + Theme::LinePadding;
	//ctx.drawPos.x = 0;

}


void ImGui::Text(String text, Color c, float scale)
{
	auto tsz = MeasureTextSize(text.c_str(), { 99999,99999 }, defaultFont);
	tsz.height *= scale;
	tsz.width *= scale;
	Text(text, tsz, c, scale);
}
void ImGui::Text(String text, Size sz, Color c, float scale)
{
	auto& ctx = *currentCotext;
	ctx.renderContext.DrawTextW(text, { ctx.drawPos.x,ctx.drawPos.y,ctx.drawPos.x + sz.width,ctx.drawPos.y + sz.height }, c, scale);

	MoveDrawpos(sz, false);
	//ctx.prevY = ctx.drawPos.y;
	//ctx.prevX = ctx.drawPos.x + sz.width + Theme::LinePadding;
	//ctx.drawPos.y += sz.height + Theme::LinePadding;
	//ctx.drawPos.x = 0;
}
bool ImGui::Button(String label, Color mask)
{
	auto& ctx = *currentCotext;
	auto tsz = MeasureTextSize(label.c_str(), { 99999,99999 }, defaultFont);
	auto p = ctx.drawPos;
	bool press, held;
	tsz.height += 10;
	tsz.width += 20;
	Rect rc = { p.x,p.y,p.x + tsz.width,p.y + tsz.height };
	bool ret = BhvrButton(HashString(label), rc, press, held);
	ctx.renderContext.DrawRect(rc, AdaptColor(Theme::Color::Background, 0.35f, 0));
	ctx.renderContext.DrawRect(rc, AdaptColor(mask, 0.35f, 0));
	if (held)
		ctx.renderContext.DrawRect(
			{
				p.x + Theme::BorderWidth,
				p.y + Theme::BorderWidth,
				p.x + tsz.width - Theme::BorderWidth,
				p.y + tsz.height - Theme::BorderWidth
			},
			ColorF(ColorF::Gray, 0.5f), press ? 0 : Theme::BorderWidth);
	ctx.renderContext.DrawTextW(label, { p.x + 10,p.y + 5,p.x + tsz.width - 10,p.y + tsz.height - 5 }, FlameUI::Theme::Color::Front);

	MoveDrawpos(tsz);
	//ctx.prevY = ctx.drawPos.y;
	//ctx.prevX = ctx.drawPos.x + tsz.width + Theme::LinePadding;
	//ctx.drawPos.y += tsz.height + Theme::LinePadding;
	//ctx.drawPos.x = 0;
	return ret;
}
bool ImGui::Button(String label, Size sz, Color mask)
{
	auto& ctx = *currentCotext;
	auto tsz = MeasureTextSize(label.c_str(), { 99999,99999 }, defaultFont);
	auto p = ctx.drawPos;
	bool press, held;
	Rect rc = { p.x,p.y,p.x + sz.width,p.y + sz.height };
	bool ret = BhvrButton(HashString(label), rc, press, held);

	ctx.renderContext.DrawRect({ p.x,p.y,p.x + sz.width,p.y + sz.height }, AdaptColor(Theme::Color::Background, 0.35f, 0));
	ctx.renderContext.DrawRect(rc, AdaptColor(mask, 0.35f, 0));
	if (held)
		ctx.renderContext.DrawRect(
			{ p.x + Theme::BorderWidth,
			p.y + Theme::BorderWidth,
			p.x + tsz.width - Theme::BorderWidth,
			p.y + tsz.height - Theme::BorderWidth },
			ColorF(ColorF::Gray, 0.5f),
			press ? 0 : Theme::BorderWidth);

	ctx.renderContext.DrawTextW(label, {
		p.x + sz.width / 2 - tsz.width / 2,
		p.y + sz.height / 2 - tsz.height / 2,
		p.x + sz.width / 2 + tsz.width / 2,
		p.y + sz.height / 2 + tsz.height / 2
		}, FlameUI::Theme::Color::Front);

	MoveDrawpos(tsz);
	//ctx.prevY = ctx.drawPos.y;
	//ctx.prevX = ctx.drawPos.x + tsz.width + Theme::LinePadding;
	//ctx.drawPos.y += tsz.height + Theme::LinePadding;
	//ctx.drawPos.x = 0;
	return ret;
}
bool ImGui::CheckBox(String label, bool& checked)
{
	auto& ctx = *currentCotext;
	float fac = Theme::LineHeight / 24.f;
	bool press, held;
	auto tsz = MeasureTextSize(label.c_str(), { 99999,99999 }, defaultFont);
	tsz.width += 22;
	Vector2& o = ctx.drawPos;
	bool clicked = BhvrButton(HashString(label), Rect(o, o + tsz), press, held);
	if (clicked) checked = !checked;
	D2D1_RECT_F shapeBack{ o.x ,o.y + 4 ,o.x + 20 * fac,o.y + 20 * fac + 4 };
	D2D1_RECT_F shapeFront{ o.x + Theme::BorderWidth,o.y + Theme::BorderWidth + 4,o.x + 20 * fac - Theme::BorderWidth,o.y + 20 * fac - Theme::BorderWidth + 4 };
	if (press)
	{
		ctx.renderContext.DrawRect(shapeBack, AdaptColor(Theme::Color::Front));
	}
	else
	{
		if (held)
		{
			if (checked)
				ctx.renderContext.DrawRect(shapeBack, Theme::Color::Theme);
			ctx.renderContext.DrawRect(shapeFront, Theme::Color::Front, Theme::BorderWidth);
		}
		else
		{
			if (checked)
				ctx.renderContext.DrawRect(shapeBack, Theme::Color::Theme);
			else
				ctx.renderContext.DrawRect(shapeFront, AdaptColor(Theme::Color::Front), Theme::BorderWidth);
		}
	}
	if (checked)
	{
		ctx.renderContext.DrawLine({ o.x + 3 * fac,o.y + 11 }, { o.x + 8 * fac,o.y + 16 }, Theme::Color::Front, 1.5 * fac);
		ctx.renderContext.DrawLine({ o.x + 8 * fac,o.y + 16 }, { o.x + 18 * fac,o.y + 6 }, Theme::Color::Front, 1.5 * fac);
	}
	ctx.renderContext.DrawTextW(label, { o.x + 22,o.y - tsz.height / 2 + 10,o.x + tsz.width,o.y + tsz.height / 2 + 10 }, Theme::Color::Front);

	MoveDrawpos(tsz);
	//ctx.prevY = ctx.drawPos.y;
	//ctx.prevX = ctx.drawPos.x + tsz.width + Theme::LinePadding;
	//ctx.drawPos.y += tsz.height > 22 ? tsz.height : 22 + Theme::LinePadding;
	//ctx.drawPos.x = 0;
	return clicked;
}
bool ImGui::CollapsingHeader(String label)
{
	auto bc = AdaptColor(Theme::Color::Background, 0.35f, 0);
	size_t p;
	auto id = HashStringSL(label);
	do
	{
		p = label.find(L"#");
		if (p != wstring::npos)
			label = label.replace(p, 1, L"");

	} while (p != wstring::npos);
	auto& ctx = *currentCotext;
	auto& r = ctx.renderContext;
	bool press, held;
	auto tsz = MeasureTextSize(label.c_str(), { 99999,99999 }, defaultFont);
	if (ctx.states.count(id) == 0)
	{
		ctx.states[id] = { 0 };
	}
	State& s = ctx.states[id];
	bool& colpsed = s.b;
	Vector2& o = ctx.drawPos;
	float h = Theme::LineHeight + Theme::LinePadding;
	Rect headerRect{ o,{ctx.currentClipRect.right,o.y + h} };
	Rect textRect{ o.x + 20,o.y + h / 2 - tsz.height / 2 ,ctx.currentClipRect.right,o.y + h / 2 + tsz.height / 2 };
	Rect signRect{ o.x + 3 ,o.y+2  ,o.x + 20,o.y + h };
	bool clicked = BhvrButton(id, headerRect, press, held);
	if (clicked) colpsed = !colpsed;
	r.DrawRect(headerRect, bc);
	if (colpsed)
		r.DrawTextW(L"🞃", signRect, Theme::Color::Front, 1.f);
	else
		r.DrawTextW(L"🞂", signRect, Theme::Color::Front, 1.f);
	r.DrawTextW(label, textRect, Theme::Color::Front);

	MoveDrawpos({ headerRect.width(),headerRect.height() });
	return colpsed;
}
