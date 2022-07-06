#include "Dropdown.h"
#include "FlameUI.h"
#include "MenuFrame.h"
#include "LinearPlacer.h"

using namespace std;
using namespace D2D1;
using namespace FlameUI;

LRESULT Dropdown::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_MOUSEENTER:
	case FE_MOUSELEAVE:
	{
		//UpdateView();
		break;
	}
	default:
		break;
	}
	return 0;
}

void Dropdown::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(Theme::Color::Front, &br);
	//if (root->mouseFoci == this || Contains(root->mouseFoci, true))
	//	br->SetColor(AdaptColor(Theme::Color::Theme));
	ctx->DrawRectangle({ 1.5f,1.5f,rect.width() - 1.5f,rect.height() - 1.5f }, br, Theme::BorderWidth);
	br->Release();
	EndDraw();
}

Dropdown::Dropdown(View* parent) :View(parent)
{
	size = { 100,30 };
	//FlameUI::Debug::showBorder = true;
	te = new TextEditBase(this);
	te->Content(L"Dropdown List");
	te->Position({ 1 + 2.f * Theme::BorderWidth,1 + 2.f * Theme::BorderWidth });
	te->Size({ 1 + 2.f * Theme::BorderWidth + Theme::LineHeight,1 + 2.f * Theme::BorderWidth });
	te->Coord(COORD_FILL, COORD_FILL);
	te->Readonly(true);
	lbTrigger = new Label(this, L"🞃");
	lbTrigger->mouseable = true;
	lbTrigger->SizeMode(SIZE_MODE_NONE, SIZE_MODE_NONE);
	lbTrigger->Coord(COORD_NEGATIVE, COORD_CENTER);
	lbTrigger->Position({ 1 + 2.f * Theme::BorderWidth ,-3 });
	lbTrigger->Size({ Theme::LineHeight,Theme::LineHeight });

	Menu* m = new Menu(L"Candidates");
	m->AppendItem(MenuItem::Common(0, nullptr, L"Candidate 1", L"", false));
	m->AppendItem(MenuItem::Common(1, nullptr, L"Candidate 2", L"", false));
	m->AppendItem(MenuItem::Common(2, nullptr, L"Candidate 3", L"", false));
	m->AppendItem(MenuItem::Common(3, nullptr, L"Candidate 4", L"", false));
	m->AppendItem(MenuItem::Common(4, nullptr, L"Candidate 5", L"", false));
	m->AppendItem(MenuItem::Common(5, nullptr, L"Candidate 6", L"", false));

	lbTrigger->AddEventListener([this, m](Message, WPARAM, LPARAM)
		{
			float ch = FlameUI::Theme::LineHeight + FlameUI::Theme::LinePadding;
			auto rc = CalcViewRectOnScreen(this);
			MenuFrame* mf = new MenuFrame((Frame*)root,
				{ (int)rc.left,(int)rc.bottom },
				{ (LONG)rect.width(),(LONG)(ch * m->items.size() + FlameUI::Theme::LinePadding * 2) });

			for (auto i : m->items)
			{
				mf->Layouter(new LinearPlacer());
				mf->Padding({ 0,Theme::LinePadding,0,0 });
				Label* lb = new Label(mf);
				lb->SizeMode(SIZE_MODE_NONE, SIZE_MODE_NONE);
				lb->Coord(COORD_FILL, COORD_POSITIVE);
				lb->Size({ 0,ch });
				lb->Position({ 0,0 });
				lb->Content(L" " + i.title);
				lb->mouseable = true;
				lb->AddEventListener([lb](Message, WPARAM, LPARAM)
					{
						lb->backgroundColor = ColorF(ColorF::Gray, 0.3f);
						lb->UpdateView();
					}, FE_MOUSEENTER);
				lb->AddEventListener([lb](Message, WPARAM, LPARAM)
					{
						lb->backgroundColor = ColorF(ColorF::Gray, 0);
						lb->UpdateView();
					}, FE_MOUSELEAVE);
				lb->AddEventListener([this, mf, lb, i](Message, WPARAM, LPARAM)
					{
						te->Content(i.title);
						SendEvent(FE_CHANGE, i.id, 0);
						mf->Close();
					}, FE_LBUTTONDOWN);

			}
			mf->Show();

		}, FE_LBUTTONDOWN);
}

void Candidates(Menu* m)
{

}