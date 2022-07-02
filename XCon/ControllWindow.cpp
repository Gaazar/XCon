#include "Frames.h"

#include "FlameUI.h"
#include "TextEditor.h"
#include "Button.h"
#include "Label.h"
#include "CheckBox.h"
#include <map>
#include <string>
#include "ScrollView.h"
#include "global.h"
#include <iostream>
#include "LinearPlacer.h"

using namespace FlameUI;
using namespace configor;

Frame* cframe = nullptr;
std::map<std::wstring, std::wstring> i18ndic
{
	{L"accelerator",L"油门"},
	{L"yaw",L"偏航"},
	{L"pitch",L"俯仰"},
	{L"roll",L"滚筒"},
	{L"toggleAccelecratorLock",L"油门锁定"},
	{L"max",L"最大值"},
	{L"min",L"最小值"},
	{L"direct",L"直接"},
	{L"integral",L"积分"},
	{L"bind",L"按键绑定"},
	{L"type",L"类型"},
	{L"record",L"录制"},

};
struct ControlSet
{
	TextEditor* name;
	TextEditor* bind;
	TextEditor* max;
	TextEditor* min;
	TextEditor* type;
	TextEditor* curve;
	CheckBox* invert;
	struct {
		TextEditor* postive;
		TextEditor* negative;
	} compositor;

	Button* btnCloaps;
	Label* lCloaps;

	Point New(wjson j, View* p, float w)
	{
		float clh = FlameUI::Theme::LineHeight * 1.3f;
		float lh = clh + FlameUI::Theme::LinePadding;
		Panel* pnl = new Panel(p);
		pnl->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_NONE);
		pnl->Size({ 0,lh });

		Label* lb;
		TextEditor* te;

		std::wstring ws;

		Point o(0, 0);

		btnCloaps = new Button(pnl);
		btnCloaps->Position(o);
		btnCloaps->Size({ 20,20 });
		//btnCloaps->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		lCloaps = new Label(btnCloaps, L"🞂");
		lCloaps->Coord(COORD_CENTER, COORD_CENTER);
		btnCloaps->AddEventListener([lh, pnl, this](Message, WPARAM, LPARAM)
			{
				if (lCloaps->Content() == L"🞂")
				{
					lCloaps->Content(L"🞃");
					pnl->Size({ 0,lh * 10 });
				}
				else
				{
					lCloaps->Content(L"🞂");
					pnl->Size({ 0,lh });
				}
			}, FE_LBUTTONUP);


		te = new TextEditor(pnl);
		lb = new Label(pnl, L"名称");
		lb->Position(o + Vector2(24, 0));
		te->Position(o + Vector2(76, 0));
		te->Multiline(false);
		te->Size({ w - 80,clh });
		if (j[L"name"].is_string())
			te->Content(j[L"name"]);
		name = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"按键绑定");
		lb->Position(o + Vector2(24, 0));
		te->Position(o + Vector2(76, 0));
		te->Multiline(false);
		te->Size({ w - 80,clh });
		if (j[L"bind"].is_string())
			te->Content(j[L"bind"]);
		bind = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"最大值");
		lb->Position(o + Vector2(24, 0));
		te->Position(o + Vector2(76, 0));
		te->Multiline(false);
		te->Size({ w - 80,clh });
		if (j[L"max"].is_string())
			te->Content(j[L"max"]);
		max = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"最小值");
		lb->Position(o + Vector2(24, 0));
		te->Position(o + Vector2(76, 0));
		te->Multiline(false);
		te->Size({ w - 80,clh });
		if (j[L"min"].is_string())
			te->Content(j[L"min"]);
		min = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"类型");
		lb->Position(o + Vector2(24, 0));
		te->Position(o + Vector2(76, 0));
		te->Multiline(false);
		te->Size({ w - 80,clh });
		if (j[L"type"].is_string())
			te->Content(j[L"type"]);
		type = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"曲线");
		lb->Position(o + Vector2(24, 0));
		te->Position(o + Vector2(76, 0));
		te->Multiline(false);
		te->Size({ w - 80,clh });
		if (j[L"curve"].is_string())
			te->Content(j[L"curve"]);
		curve = te;

		o = o + Vector2(0, lh);
		lb = new Label(pnl, L"反转");
		lb->Position(o + Vector2(24, 0));
		CheckBox* cb = new CheckBox(pnl);
		cb->Position(o + Vector2(76, 0));
		invert = cb;

		o = o + Vector2(0, clh);
		lb = new Label(pnl, L"合成");
		lb->Position(o + Vector2(24, 0));


		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"+");
		lb->Position(o + Vector2(24 + 24, 0));
		te->Position(o + Vector2(76 + 24, 0));
		te->Multiline(false);
		te->Size({ w - 80 - 24,clh });
		if (j[L"compositor"].is_object() && j[L"compositor"][L"+"].is_string())
			te->Content(j[L"compositor"][L"+"]);
		compositor.postive = te;


		o = o + Vector2(0, lh);
		te = new TextEditor(pnl);
		lb = new Label(pnl, L"-");
		lb->Position(o + Vector2(24 + 24, 0));
		te->Position(o + Vector2(76 + 24, 0));
		te->Multiline(false);
		te->Size({ w - 80 - 24,clh });
		if (j[L"compositor"].is_object() && j[L"compositor"][L"-"].is_string())
			te->Content(j[L"compositor"][L"-"]);
		compositor.negative = te;

		o = o + Vector2(0, lh);

		return o;
	}

};

std::vector<ControlSet*> csets;
void ShowControlWindow()
{
	if (cframe)
	{
		FlashWindow(cframe->GetNative(), FALSE);
		return;
	}
	cframe = new Frame({ 400,500 });
	cframe->Title(L"按键绑定");
	cframe->sizable = false;
	cframe->maximal = false;

	ScrollView* sv = new ScrollView(cframe);
	sv->Position({ 1,40 });
	sv->Size({ 1,50 });
	sv->Coord(COORD_FILL, COORD_FILL);

	sv->Content()->Padding({ 10,0,10,0 }, true);
	//

	Panel* bpnl = new Panel(cframe);
	bpnl->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	bpnl->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	bpnl->Position({ 15,8 });
	bpnl->Layouter(new LinearPlacer(DIRECTION_HORIZONTAL, 5));

	Button* btn = new Button(bpnl);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"取消");

	btn = new Button(bpnl);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"保存");

	btn = new Button(bpnl);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"应用");


	wjson contrs = configs[L"control"];
	Point o = { 0,0 };
	sv->Content()->Layouter(new LinearPlacer(DIRECTION_VERTICAL, 8));
	for (auto i : contrs)
	{
		ControlSet* cs = new ControlSet();
		auto t = GetTime(1000000);
		cs->New(i, sv->Content(), 398 - 20 - 15);
		//std::cout << GetTime(1000000) - t << "us" << std::endl;
		csets.push_back(cs);
	}

	cframe->AddEventListener([](Message, WPARAM, LPARAM) {
		cframe = nullptr;
		}, FE_DESTROY);

	cframe->Show();


}