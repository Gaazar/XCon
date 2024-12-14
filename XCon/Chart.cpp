#include "Chart.h"
#include "FlameUI.h"


using namespace std;
using namespace D2D1;
using namespace FlameUI;

LRESULT Chart::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == FE_SIZED)
	{
		for (int i = 0; i < series.size(); i++)
		{
			GenerateCurve(i);
		}
	}
	return 0;
}
void Chart::Draw()
{
	auto ctx = BeginDraw(ColorF(ColorF::Black, 0));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(FlameUI::Theme::Color::Front, &br);
	ctx->DrawLine({ 0,rect.height() / 2 }, { rect.width(), rect.height() / 2 }, br);
	br->SetOpacity(0.5f);
	ctx->DrawLine({ 0,20 }, { rect.width(), 20 }, br);
	ctx->DrawTextW(toStringW(gmax).c_str(), toStringW(gmax).length(), root->dTextFormat, { 0.f,20.f,1000.f,40.f }, br);
	ctx->DrawLine({ 0,rect.height() - 20 }, { rect.width(), rect.height() - 20 }, br);
	ctx->DrawTextW(toStringW(-gmax).c_str(), toStringW(-gmax).length(), root->dTextFormat, { 0.f,rect.height() - 20,1000.f,rect.height() }, br);

	br->SetOpacity(1);


	for (int i = 0; i < rect.height() / 2 / 50; i++)
	{

	}
	for (auto i : series)
	{
		if (i.path)
		{
			br->SetColor(i.color);
			ctx->DrawGeometry(i.path, br);
		}
	}
	for (int i = 0; i < series.size(); i++)
	{
		br->SetColor(series[i].color);
		ctx->DrawTextW(series[i].name.c_str(), series[i].name.length(), root->dTextFormat, { 60.f,FlameUI::Theme::LineHeight * i,1000.f,FlameUI::Theme::LineHeight * (i + 1) }, br);
	}
	br->Release();
	EndDraw();
}

Chart::Chart(View* parent) :View(parent)
{
	//FlameUI::Debug::showBorder = true;
	//aid = Animate(16, 0, 0);
}
void Chart::JoinSeries(std::wstring name, Color color)
{
	series.push_back(Series());
	Series& s = series[series.size() - 1];
	s.name = name;
	s.color = color;
}

void Chart::JoinValue(int i, float value, bool generateCurve)
{
	series[i].values.push_back(value);
	if (generateCurve) GenerateCurve(i);

}
void Chart::GenerateCurve(int i)
{
	//int l = series[i].values.size() - 1;
	int vl = series[i].values.size();
	if (series[i].path)
	{
		series[i].path->Release();
		series[i].path->Release();
	}
	int r = maxHistory;
	float max = 0.001f;
	for (int j = 0; j < series.size(); j++)
	{
		float tmax = 0.001f;
		int vl = series[j].values.size();
		if (beginIndex < 0)
		{
			for (int n = 0; n < maxHistory; n++)
			{
				if (vl - n - 1 < 0) break;
				if (abs(series[j].values[vl - n - 1]) > tmax)
					tmax = abs(series[j].values[vl - n - 1]);

			}
		}
		else
		{
			for (int n = 0; n < maxHistory; n++)
			{
				int c = maxHistory - n - 1;
				if (beginIndex + n >= vl) break;

				if (abs(series[j].values[beginIndex + n]) > tmax)
					tmax = abs(series[j].values[beginIndex + n]);

			}
		}
		if (tmax > max) max = tmax;
	}

	gmax = max;
	float w = rect.width();
	float h = rect.height();
	float wr = w / (r - 1);
	float hr = (h - 40) / max / 2;
	gD2DFactory->CreatePathGeometry(&series[i].path);
	ID2D1GeometrySink* s;
	auto hrr = series[i].path->Open(&s);
	if (beginIndex < 0)
	{
		for (int n = 0; n < maxHistory; n++)
		{
			if (vl - n - 1 < 0) break;
			if (n == 0)
			{
				s->BeginFigure({ w - wr * n ,h / 2 - series[i].values[vl - n - 1] * hr }, D2D1_FIGURE_BEGIN_HOLLOW);
			}
			else
			{
				s->AddLine({ w - wr * n ,h / 2 - series[i].values[vl - n - 1] * hr });
			}
		}
	}
	else
	{
		for (int n = 0; n < maxHistory; n++)
		{
			int c = maxHistory - n - 1;
			if (beginIndex + n >= vl)
				break;
			if (n == 0)
			{
				s->BeginFigure({ w - wr * c ,h / 2 - series[i].values[beginIndex + n] * hr }, D2D1_FIGURE_BEGIN_HOLLOW);
			}
			else
			{
				s->AddLine({ w - wr * c ,h / 2 - series[i].values[beginIndex + n] * hr });
			}
		}
	}
	s->EndFigure(D2D1_FIGURE_END_OPEN);
	hrr = s->Close();
	if (hrr != S_OK)
	{
		s->Release();
		series[i].path->Release();
		series[i].path = nullptr;
	}
	UpdateView();
}

void Chart::SetValueSet(int i, vector<float>& values)
{
	series[i].values = values;
}

void Chart::Animation(float progress, int p1, int p2)
{


}
