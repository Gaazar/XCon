#include "global.h"
#include "FlameUIBasics.h"

using namespace FlameUI;
using namespace configor;

Frame* calbframe = nullptr;

struct Prop
{
	std::wstring propname;
	TextEditor* value;
};
struct PropMap
{
	std::wstring name;
	std::vector<Prop> props;
	PropMap(std::wstring n)
	{
		name = n;
	}
};

std::vector<PropMap> propmaps;

void CalibrateShow()
{
	if (calbframe)
	{
		FlashWindow(calbframe->GetNative(), FALSE);
		return;
	}
	calbframe = new Frame({ 400,500 });
	calbframe->Title(L"校准调零");
	calbframe->sizable = false;
	calbframe->maximal = false;

	ScrollView* sv = new ScrollView(calbframe);
	sv->Position({ 1,40 });
	sv->Size({ 1,50 });
	sv->Coord(COORD_FILL, COORD_FILL);

	sv->Content()->Padding({ 10,0,10,0 }, true);
	//

	Panel* bpnl = new Panel(calbframe);
	bpnl->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	bpnl->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	bpnl->Position({ 15,8 });
	bpnl->Layouter(new LinearPlacer(DIRECTION_HORIZONTAL, 5));

	Button* btn = new Button(bpnl);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"加载");

	btn = new Button(bpnl);
	btn->Padding({ 10,5,10,5 });
	btn->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"保存");
	btn->AddEventListener([](Message, WPARAM, LPARAM)
		{
			for (auto i : propmaps)
			{
				for (auto n : i.props)
				{
					configs[L"calibration"][i.name][n.propname] = n.value->Content();
				}
			}
			SaveConfigs();
		}, FE_LBUTTONDOWN);


	wjson conf = configs[L"calibration"];
	Point o = { 0,0 };
	sv->Content()->Layouter(new LinearPlacer(DIRECTION_VERTICAL, 8));
	auto svc = sv->Content();
	int i = 0;
	//svc->Coord(COORD_FILL, COORD_POSITIVE);

	float clh = FlameUI::Theme::LineHeight * 1.3f;
	float lh = clh + FlameUI::Theme::LinePadding;
	//FlameUI::Debug::showBorder = true;
	for (auto iter = conf.begin(); iter != conf.end(); iter++)
	{
		auto l = new Label(svc, iter.key());
		propmaps.push_back(PropMap(iter.key()));
		l->Font(GetFont(L"", 16));

		for (auto nter = iter.value().begin(); nter != iter.value().end(); nter++)
		{
			l = new Label(svc, nter.key());
			l->Position({ 20,0 });
			l->SizeMode(SIZE_MODE_NONE, SIZE_MODE_NONE);
			//l->Coord(COORD_FILL, COORD_POSITIVE);
			l->Size({ 400 - 10 - 20 - 15 ,clh });
			auto te = new TextEditor(l);
			te->Position({ 10,0 });
			te->Coord(COORD_NEGATIVE, COORD_POSITIVE);
			te->Multiline(false);
			te->Size({ 250,clh });
			te->Content(nter.value().as_string());

			propmaps[i].props.push_back({ nter.key(), te });
		}


		i++;
	}


	calbframe->AddEventListener([](Message, WPARAM, LPARAM) {
		calbframe = nullptr;
		}, FE_DESTROY);

	calbframe->Show();

}