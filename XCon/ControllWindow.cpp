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

	Point New(wjson j, View* p, Point o, float w)
	{
		float clh = FlameUI::Theme::LineHeight * 1.3f;
		float lh = clh + FlameUI::Theme::LinePadding;
		Label* lb;
		TextEditor* te;

		std::wstring ws;

		te = new TextEditor(p);
		lb = new Label(p, L"名称");
		lb->Position(o);
		te->Position(o + Vector2(70, 0));
		te->Multiline(false);
		te->Size({ w - 75,clh });
		if (j[L"name"].is_string())
			te->Content(j[L"name"]);
		name = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"按键绑定");
		lb->Position(o);
		te->Position(o + Vector2(70, 0));
		te->Multiline(false);
		te->Size({ w - 75,clh });
		if (j[L"bind"].is_string())
			te->Content(j[L"bind"]);
		bind = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"最大值");
		lb->Position(o);
		te->Position(o + Vector2(70, 0));
		te->Multiline(false);
		te->Size({ w - 75,clh });
		if (j[L"max"].is_string())
			te->Content(j[L"max"]);
		max = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"最小值");
		lb->Position(o);
		te->Position(o + Vector2(70, 0));
		te->Multiline(false);
		te->Size({ w - 75,clh });
		if (j[L"min"].is_string())
			te->Content(j[L"min"]);
		min = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"类型");
		lb->Position(o);
		te->Position(o + Vector2(70, 0));
		te->Multiline(false);
		te->Size({ w - 75,clh });
		if (j[L"type"].is_string())
			te->Content(j[L"type"]);
		type = te;

		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"曲线");
		lb->Position(o);
		te->Position(o + Vector2(70, 0));
		te->Multiline(false);
		te->Size({ w - 75,clh });
		if (j[L"curve"].is_string())
			te->Content(j[L"curve"]);
		curve = te;

		o = o + Vector2(0, lh);
		lb = new Label(p, L"反转");
		lb->Position(o);
		CheckBox* cb = new CheckBox(p);
		cb->Position(o + Vector2(70, 0));
		invert = cb;

		o = o + Vector2(0, clh);
		lb = new Label(p, L"合成");
		lb->Position(o);


		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"+");
		lb->Position(o + Vector2(30, 0));
		te->Position(o + Vector2(100, 0));
		te->Multiline(false);
		te->Size({ w - 105,clh });
		if (j[L"compositor"].is_object() && j[L"compositor"][L"+"].is_string())
			te->Content(j[L"compositor"][L"+"]);
		compositor.postive = te;


		o = o + Vector2(0, lh);
		te = new TextEditor(p);
		lb = new Label(p, L"-");
		lb->Position(o + Vector2(30, 0));
		te->Position(o + Vector2(100, 0));
		te->Multiline(false);
		te->Size({ w - 105,clh });
		if (j[L"compositor"].is_object() && j[L"compositor"][L"-"].is_string())
			te->Content(j[L"compositor"][L"-"]);
		compositor.negative = te;

		o = o + Vector2(0, lh);

		return o;
	}

};

std::vector<ControlSet> csets;
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

	Button* btn = new Button(cframe);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->Position({ 5,5 });
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"取消");

	btn = new Button(cframe);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->Position({ 5 + 65,5 });
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"保存");

	btn = new Button(cframe);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->Position({ 5 + 65 * 2,5 });
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"应用");


	wjson contrs = configs[L"control"];
	Point o = { 0,0 };
	for (auto i : contrs)
	{
		ControlSet cs;
		auto t = GetTime(1000000);
		o = cs.New(i, sv->Content(), o, 398 - 20 - 15);
		//std::cout << GetTime(1000000) - t << "us" << std::endl;
		csets.push_back(cs);
		o = o + Vector2(0, FlameUI::Theme::LineHeight);
	}

	cframe->AddEventListener([](Message, WPARAM, LPARAM) {
		cframe = nullptr;
		}, FE_DESTROY);

	cframe->Show();


}