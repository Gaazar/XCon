#include "DockProvider.h"
#include "FlameUI.h"
#include <iostream>
using namespace FlameUI;
using namespace D2D1;

void DrawCross(ID2D1DeviceContext* ctx, D2D1_POINT_2F center, float size, float strokew, ID2D1Brush* br)
{
	ctx->DrawLine({ center.x - size,center.y - size }, { center.x + size,center.y + size }, br, strokew);
	ctx->DrawLine({ center.x + size,center.y - size }, { center.x - size,center.y + size }, br, strokew);
}
void DockProvider::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	float left = 0;
	float h = Theme::LineHeight + Theme::LinePadding;
	ID2D1SolidColorBrush* adp, * theme, * front;
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Background, 0.35f, 0), &adp);
	ctx->CreateSolidColorBrush(Theme::Color::Theme, &theme);
	ctx->CreateSolidColorBrush(Theme::Color::Front, &front);
	int n = 0;
	//std::cout << current << std::endl;
	for (auto i : tabs)
	{
		if (current == n)
		{
			if (press >= 0)
			{
				ctx->FillRectangle({ left,0.f,left + i.length,Theme::LinePadding / 2.f }, theme);
			}
			else
			{
				ctx->FillRectangle({ left,0.f,left + i.length,Theme::LinePadding / 2.f }, theme);
			}
		}
		else
		{
			ctx->FillRectangle({ left,0.f,left + i.length,Theme::LineHeight + Theme::LinePadding * 1.5f }, adp);

		}
		ctx->DrawTextW(i.name.c_str(), i.name.length(), root->dTextFormat,
			{ left + Theme::LinePadding ,Theme::LinePadding / 2.f,
			left + i.length,Theme::LineHeight + Theme::LinePadding * 1.5f }, front);
		if (miIdx == n)
			DrawCross(ctx, { left + i.length - 10.f, Theme::LinePadding / 2.f + 0.5f * (Theme::LineHeight + Theme::LinePadding * 0.75f) }, 4, 1.2f, front);


		left += i.length;
		n++;
	}
	adp->Release();
	theme->Release();
	front->Release();
	EndDraw();
}
LRESULT DockProvider::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_MOUSEHIT:
	{
		D2D1_POINT_2F& mouse = *(D2D1_POINT_2F*)wParam;
		if (mouse.y > Theme::LineHeight + Theme::LinePadding) return 1;
		if (tabs.size() == 1 && root->mainDockProvider)
		{
			docking = true;
			if (mouse.x >= tabs[0].length - 15 && mouse.x < tabs[0].length)
			{
				miIdx = 0;
				UpdateView();
				return 1;
			}
			return 0;
		}
		float twid = 0;
		int n = 0;
		for (auto i = tabs.begin(); i != tabs.end(); i++)
		{
			if (mouse.x > twid && mouse.x < twid + i->length && mouse.y < Theme::LineHeight + Theme::LinePadding)
			{
				miIdx = n;
				UpdateView();
			}
			twid += i->length;
			n++;
		}
		if (mouse.x < twid && mouse.y < Theme::LineHeight + Theme::LinePadding * 1.5f)
		{
			return 1;
		}

		if (miIdx != -1)
			UpdateView();
		miIdx = -1;
		return 0;
	}
	case FE_MOUSELEAVE:
		UpdateView();
		miIdx = -1;
		break;
	case FE_LBUTTONUP:
	{
		press = -1;
		float twid = 0;
		int n = 0;
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);

		for (auto i : tabs)
		{

			if (x >= twid + i.length - 15 && x < twid + i.length && y < Theme::LineHeight + Theme::LinePadding)
			{
				cout << "close" << endl;
				bool revert = false;
				current = n - 1;
				if (tabs[n].content)
					tabs[n].content->Destroy();
				if (current < 0) current = 0;
				tabs.erase(tabs.begin() + n);
				if (tabs.size() > 0)
				{
					if (tabs[current].content)
					{
						tabs[current].content->Alpha(0);
						tabs[current].content->disabled = false;
					}
				}
				else
				{
					if (root->mainDockProvider)
					{
						root->mainDockProvider->ClearTabs();
						root->Destroy();
						return 0;
					}
					else if (root->GetChildren()[0] == this)
					{
						root->Destroy();
						return 0;
					}
					RunInUIThread([this]()
						{
							auto basePanel = this->parent->Parent();
							if (basePanel)
								for (auto i : basePanel->GetChildren())
								{
									if (i != parent)
									{
										for (auto n : i->GetChildren())
										{
											n->Parent(basePanel);
										}
									}
									i->Destroy();

								}
						});
				}

			}

			twid += i.length;
			n++;
		}
		break;
	}
	case FE_LBUTTONDOWN:
	{
		float twid = 0;
		int n = 0;
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		for (auto i : tabs)
		{
			if (x > twid && x < twid + i.length && y < Theme::LineHeight + Theme::LinePadding)
			{
				if (tabs[current].content)
					tabs[current].content->Alpha(1), tabs[current].content->disabled = true;
				if (tabs[n].content)
					tabs[n].content->Alpha(0), tabs[n].content->disabled = false;
				press = n;
				current = n;
				pressOffset.y = y;
				pressOffset.x = x - twid;
			}
			twid += i.length;
			n++;
		}
		UpdateView();
		//std::cout << current << "||" << x << "," << y << std::endl;
	}
	case FE_MOUSEMOVE:
	{
		float twid = 0;
		int n = 0;
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		if (y > Theme::LineHeight + Theme::LinePadding && press < 0)
			return (LRESULT)root;
		mpos.x = x;
		mpos.y = y;
		if (press >= 0)
		{
			if (y < 0 || y > Theme::LineHeight + Theme::LinePadding || x < 0 || x > rect.width())
			{
				auto tab = tabs[current];
				bool revert = false;
				current--;
				if (current < 0) current = 0;
				tabs.erase(tabs.begin() + press);
				press = -1;
				if (tabs.size() > 0)
				{
					if (tabs[current].content)
						tabs[current].content->Alpha(0), tabs[current].content->disabled = false;
				}
				else
				{
					revert = true;
				}
				UpdateView();
				RunInMainThread([this, tab, x, y, revert]()
					{//Mem leak: old DockProvider
						auto rc = CalcViewRectOnScreen(this);
						POINT ps = { rc.left,rc.top };
						LONG rx = (LONG)x + ps.x;
						LONG ry = (LONG)y + ps.y;
						ReleaseCapture();
						Frame* next = new Frame({ (LONG)rect.width() ,(LONG)rect.height() }, { (rx - (LONG)pressOffset.x),(ry - (LONG)pressOffset.y) });
						next->close = false;
						next->titled = false;
						next->Show();
						next->mousepos.x = pressOffset.x;
						next->mousepos.y = pressOffset.y;

						//PostMessage(next->GetNative(), WM_NCMOUSEMOVE, 0, ((short)((rx + pressOffset.x * dpiScaleFactor.x)) << 16 | (short)(ry * dpiScaleFactor.y)));
						//PostMessage(next->GetNative(), WM_NCMOUSEMOVE, 0, 0x000f0001);
						PostMessage(next->GetNative(), WM_NCLBUTTONDOWN, HTCAPTION, 0);
						auto dp = new DockProvider(next);
						tab.content->Parent(dp);
						dp->AddTab(tab.name, tab.content);
						dp->docking = true;
						if (revert)
						{
							RunInUIThread([this]()
								{
									auto basePanel = this->parent->Parent();
									if (basePanel)
										for (auto i : basePanel->GetChildren())
										{
											if (i != parent)
											{
												for (auto n : i->GetChildren())
												{
													n->Parent(basePanel);
												}
											}
											i->Destroy();

										}
								});

						}

						UpdateView();
					});
			}
			else
			{
				for (auto i = tabs.begin(); i != tabs.end(); i++)
				{
					if (x > twid && x < twid + i->length && y < Theme::LineHeight + Theme::LinePadding)
					{
						if (n != press)
						{
							auto tab = tabs[n];
							tabs[n] = tabs[press];
							tabs[press] = tab;
							current = n;
							press = n;
							break;
						}
					}
					twid += i->length;
					n++;
				}
				UpdateView();

			}
		}
	}


	}

	return 0;
}
DockProvider::DockProvider(View* parent) :View(parent)
{
	render.direct = true;
	monitor = true;
	custommousetest = true;
	Coord(COORD_FILL, COORD_FILL);
	Size({ 0,0 });
	_addDockProvider(this);
#ifdef _DEBUG
	{
		if (root->mainDockProvider)
		{
			OutputDebugStringW(L"Exist Dock Provider\n");
		}
	}
#endif
	root->mainDockProvider = this;
	//tabs.push_back({ L"Properties",nullptr,100.f });
	//tabs.push_back({ L"Lightings",nullptr,94.f });
	//tabs.push_back({ L"Geometry",nullptr,90.f });
}
void DockProvider::AddTab(wstring name, Panel* content)
{
	docking = false;
	auto r = MeasureTextSize(name.c_str(), { 9999,9999 }, root->dTextFormat);
	tabs.push_back({ name,content,r.width + Theme::LineHeight * 2.f });
	if (tabs.size() != 1 && content) content->Alpha(1);

}

Panel* DockProvider::AddTab(wstring name)
{
	docking = false;
	auto r = MeasureTextSize(name.c_str(), { 9999,9999 }, root->dTextFormat);
	Panel* content;
	content = new Panel(this);
	content->Coord(COORD_FILL, COORD_FILL);
	content->Size({ 0,0 });
	content->Position({ 0,Theme::LineHeight + Theme::LinePadding });
	tabs.push_back({ name,content,r.width + Theme::LineHeight * 2.f });
	if (tabs.size() != 1 && content) content->Alpha(1), content->disabled = true;
	return content;
}
DockProvider::~DockProvider()
{
	_removeDockProvider(this);
}
void DockProvider::PreviewBegin()
{
	//std::cout << "Beign" << std::endl;
	if (previewer) previewer->SendEvent(FE_DESTROY, 0, 0);
	previewer = new Panel(this);
	previewer->Coord(COORD_FILL, COORD_FILL);
	previewer->Size({ 0, 0 });
	previewer->background = Theme::Color::Theme;
	previewer->background.a = 0.5f;
}
void DockProvider::PreviewUpdate(Point cursor)
{
	if (previewer)
	{
		if (cursor.x < 0.3333f * rect.width())
		{
			previewer->Position({ 0, 0 });
			previewer->Size({ 0.6667f * rect.width(), 0 });
			placeSide = SIDE_LEFT;
		}
		else if (cursor.x < 0.6667f * rect.width())
		{
			if (cursor.y < 0.3333f * rect.height())
			{
				previewer->Position({ 0, 0 });
				previewer->Size({ 0, 0.6667f * rect.height() });
				placeSide = SIDE_TOP;

			}
			else if (cursor.y < 0.6667f * rect.height())
			{
				previewer->Position({ 0, 0 });
				previewer->Size({ 0, 0 });
				placeSide = SIDE_CENTER;

			}
			else
			{
				previewer->Position({ 0, 0.6667f * rect.height() });
				previewer->Size({ 0, 0 });
				placeSide = SIDE_BOTTOM;

			}
		}
		else
		{
			previewer->Position({ 0.6667f * rect.width(), 0 });
			previewer->Size({ 0, 0 });
			placeSide = SIDE_RIGHT;
		}
	}
	//std::cout << "Update" << std::endl;
}

void DockProvider::PreviewEnd(bool place, Table tab)
{
	//std::cout << "End" << std::endl;
	if (previewer)
	{
		if (place)
		{
			previewer->background.a = 0;
			previewer->Parent(parent);
			switch (placeSide)
			{
			case FlameUI::SIDE_LEFT:
			{

				SeperatorHandle* hdl = new SeperatorHandle(parent);
				Panel* np = new Panel(parent);
				np->Coord(COORD_FILL, COORD_FILL);
				np->Size({ 0, 0 });
				np->Position({ previewer->GetRect().right + FlameUI::Theme::LinePadding,position.y });

				previewer->Size({ previewer->GetRect().width(), previewer->Size().height });
				previewer->Coord(COORD_POSTIVE, COORD_FILL);
				hdl->Position({ previewer->GetRect().right ,0 });
				hdl->BindEdge(EDGE_RIGHT, previewer);
				hdl->BindEdge(EDGE_LEFT, np);


				this->Parent(np);
				auto dp = new DockProvider(previewer);
				np->value = (long long)(new DockInfo(false, this));
				previewer->value = (long long)(new DockInfo(false, dp));
				dp->AddTab(tab.name, tab.content);
				tab.content->Parent(dp);
				root->mainDockProvider = nullptr;
				if (parent->Parent())
				{
					View* basePanel = parent->Parent();
					basePanel->value = (long long)(new DockInfo(true, np, previewer, hdl));
				}

				break;
			}
			case FlameUI::SIDE_TOP:
			{
				SeperatorHandle* hdl = new SeperatorHandle(parent, true);
				Panel* np = new Panel(parent);
				np->Coord(COORD_FILL, COORD_FILL);
				np->Size({ 0, 0 });
				np->Position({ position.x,previewer->GetRect().bottom + FlameUI::Theme::LinePadding });

				previewer->Size({ previewer->Size().width,previewer->GetRect().height() });
				previewer->Coord(COORD_FILL, COORD_POSTIVE);
				hdl->Position({ 0 ,previewer->GetRect().bottom });
				hdl->BindEdge(EDGE_BOTTOM, previewer);
				hdl->BindEdge(EDGE_TOP, np);

				this->Parent(np);
				auto dp = new DockProvider(previewer);
				np->value = (long long)(new DockInfo(false, this));
				previewer->value = (long long)(new DockInfo(false, dp));
				dp->AddTab(tab.name, tab.content);
				tab.content->Parent(dp);
				root->mainDockProvider = nullptr;
				if (parent->Parent())
				{
					View* basePanel = parent->Parent();
					basePanel->value = (long long)(new DockInfo(true, np, previewer, hdl));
				}

				break;
			}
			case FlameUI::SIDE_RIGHT:
			{
				SeperatorHandle* hdl = new SeperatorHandle(parent);
				Panel* np = new Panel(parent);
				np->Coord(COORD_POSTIVE, COORD_FILL);
				np->Size({ previewer->GetRect().left - FlameUI::Theme::LinePadding, 0 });
				np->Position({ 0,0 });

				hdl->Position({ previewer->GetRect().left - FlameUI::Theme::LinePadding,0 });
				hdl->BindEdge(EDGE_RIGHT, np);
				hdl->BindEdge(EDGE_LEFT, previewer);

				this->Parent(np);
				auto dp = new DockProvider(previewer);
				np->value = (long long)(new DockInfo(false, this));
				previewer->value = (long long)(new DockInfo(false, dp));
				dp->AddTab(tab.name, tab.content);
				tab.content->Parent(dp);
				root->mainDockProvider = nullptr;
				if (parent->Parent())
				{
					View* basePanel = parent->Parent();
					basePanel->value = (long long)(new DockInfo(true, np, previewer, hdl));
				}

				break;
			}
			case FlameUI::SIDE_BOTTOM:
			{
				SeperatorHandle* hdl = new SeperatorHandle(parent, true);
				Panel* np = new Panel(parent);
				np->Coord(COORD_FILL, COORD_POSTIVE);
				np->Size({ 0,previewer->GetRect().top - FlameUI::Theme::LinePadding });
				np->Position({ 0,0 });

				hdl->Position({ 0, previewer->GetRect().top - FlameUI::Theme::LinePadding });
				hdl->BindEdge(EDGE_BOTTOM, np);
				hdl->BindEdge(EDGE_TOP, previewer);


				this->Parent(np);
				auto dp = new DockProvider(previewer);
				np->value = (long long)(new DockInfo(false, this));
				previewer->value = (long long)(new DockInfo(false, dp));
				dp->AddTab(tab.name, tab.content);
				tab.content->Parent(dp);
				root->mainDockProvider = nullptr;
				if (parent->Parent())
				{
					View* basePanel = parent->Parent();
					basePanel->value = (long long)(new DockInfo(true, np, previewer, hdl));
				}

				break;
			}
			case FlameUI::SIDE_CENTER:
			{
				tab.content->Parent(this);
				AddTab(tab.name, tab.content);
				auto n = tabs.size() - 1;
				if (tabs[current].content)
					tabs[current].content->Alpha(1), tabs[current].content->disabled = true;
				if (tabs[n].content)
					tabs[n].content->Alpha(0), tabs[n].content->disabled = false;
				current = n;
				previewer->Destroy();
				UpdateView();
				break;
			}
			default:
				break;
			}
		}
		else
			previewer->Destroy();
	}
	previewer = nullptr;
	UpdateView();
}
DockProvider::Table DockProvider::GetPlaceTab()
{
	if (tabs.size() == 1)
	{
		return tabs[0];
	}
	return {};
}

void DockProvider::ClearTabs()
{
	tabs.clear();
}
