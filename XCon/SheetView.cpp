#include "SheetView.h"
#include "FlameUI.h"
using namespace FlameUI;
using namespace D2D1;

LRESULT SheetView::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_S_SCROLLING:
	{
		UpdateView();
		break;
	}
	case FE_SCROLL:
	{
		//std::cout << "svs" << std::endl;
		return scrl->SendEvent(msg, wParam, FE_S_L_FROMPARENT);
		break;
	}
	case FE_LBUTTONDOWN:
	{
		float ofs = scrl->Offset() + GET_Y_LPARAM(lParam) - 1.8f * Theme::LineHeight;
		int bias = ofs / heightCol;
		if (bias >= 0 && bias < data.size())
		{
			selected = bias;
			UpdateView();
			SendEvent(SVE_SELECT, selected, 0);
		}
		break;
	}
	default:
		break;
	}
	return 0;

}
void SheetView::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));


	ID2D1SolidColorBrush* br = nullptr;
	ctx->CreateSolidColorBrush(Theme::Color::Front, &br);
	float ofs = scrl->Offset();
	int bias = ofs / heightCol;
	int cnt = (rect.height() - 1.8f * Theme::LineHeight / (int)heightCol) + 2;
	float w = rect.width();
	if (br)
	{
		for (auto i = titles.begin(); i != titles.end(); ++i)
		{
			ctx->DrawTextW(i->display.c_str(), i->display.length(), fmt_ttl,
				{ (i->p) * w + 10,0,(i->p + i->w) * w, 1.8f * Theme::LineHeight }
			, br, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		}
		ctx->PushAxisAlignedClip({ 0, 1.8f * Theme::LineHeight, w, rect.height() }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		for (int i = 0; i < cnt; i++)
		{
			int n = bias + i;
			float h = 1.8f * Theme::LineHeight + heightCol * n - ofs;
			br->SetColor(ColorF::ColorF(ColorF::Gray));
			br->SetOpacity(0.2f);
			if (!(n % 2))
				ctx->FillRectangle({ 0, h, w, h + heightCol }, br);
			br->SetColor(Theme::Color::Front);
			br->SetOpacity(1);
			if (n < data.size())
			{
				for (auto j = data[n].begin(); j != data[n].end(); j++)
				{
					int idx = j - data[n].begin();
					ctx->DrawTextW(j->disp.c_str(), j->disp.length(), fmt_cnt,
						{ w * titles[idx].p + 10, h, w * (titles[idx].p + titles[idx].w), h + heightCol }
					, br, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);

				}
				if (n == selected)
				{
					br->SetColor(Theme::Color::Theme);
					br->SetOpacity(0.05f);
					ctx->FillRectangle({ 0, h, w, h + heightCol }, br);
				}
			}
		}
		ctx->PopAxisAlignedClip();
		br->SetColor(ColorF::ColorF(ColorF::Gray));
		br->SetOpacity(0.8f);
		for (auto i = titles.begin(); i != titles.end(); ++i)
		{
			ctx->DrawLine({ w * (i->p + i->w) , 0 }, { w * (i->p + i->w), rect.height() }, br);
		}
		br->Release();
	}

	EndDraw();
}

SheetView::SheetView(View* parent) :View(parent)
{
	heightCol = Theme::LineHeight + Theme::LinePadding;
	wchar_t locale[85];
	GetUserDefaultLocaleName(locale, sizeof(locale));
	gDWFactory->CreateTextFormat(L"",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Theme::BasicTextSize,
		locale,
		&fmt_cnt
	);
	fmt_cnt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	//fmt_cnt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	gDWFactory->CreateTextFormat(L"",
		NULL,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Theme::BasicTextSize,
		locale,
		&fmt_ttl
	);
	fmt_ttl->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	//fmt_ttl->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	scrl = new Scroller(this);
	scrl->Coord(COORD_NEGATIVE, COORD_FILL);
	scrl->Position({ 0,1.8f * Theme::LineHeight });
	scrl->Size({ 15, 0 });
	Size({ 100, 100 });

	keyable = true;
}
SheetView* SheetView::AddTitle(wstring display, float weight)
{
	totalWeight += weight;
	titles.push_back({ display,weight, 0,0 });
	float sum = 0;
	for (auto i = titles.begin(); i != titles.end(); i++)
	{
		i->p = sum;
		i->w = i->weight / totalWeight;
		sum += i->weight / totalWeight;
	}
	return this;
}
void SheetView::AddRow(initializer_list<Item> data)
{
	vector<Item> col(data.begin(), data.end());
	this->data.push_back({ col });
	scrl->Max(this->data.size() * heightCol);
	UpdateView();
}
void SheetView::Modify(int row, initializer_list<Item> data)
{
	if (row < 0 || row >= this->data.size()) return;
	this->data[row] = vector<Item>(data.begin(), data.end());
	UpdateView();


}
void SheetView::Modify(int row, int col, Item data)
{
	if (row < 0 || row >= this->data.size()) return;
	if (col < 0 || col >= titles.size()) return;
	if (col > this->data[row].size())
		this->data[row].resize(titles.size());
	this->data[row][col] = data;
	UpdateView();
}

void SheetView::Clear()
{
	selected = -1;
	data.clear();
	UpdateView();
}



Item* SheetView::Selection(int row, int col)
{
	if (row < 0 || row >= data.size()) return 0;
	if (col < 0 || col >= data[row].size()) return 0;
	return &data[row][col];
}