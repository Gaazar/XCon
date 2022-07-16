#pragma once
#include "View.h"
namespace FlameUI
{
	class Chart :
		public View
	{
	private:
		struct Series
		{
			std::vector<float> values;
			Color color;
			ID2D1PathGeometry* path;
			std::wstring name;
		};
		std::vector<Series> series;
		animation_id aid;
		float gmax;
		int imax;
		float dt = 0;

		void Animation(float progress, int p1, int p2) override;
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
	public:
		int maxHistory = 300;
		int beginIndex = -1;
		Chart(View* parent);
		void SetValueSet(int series, vector<float>& values);
		void JoinSeries(std::wstring name, Color color);
		void JoinValue(int series, float value,bool generateCurve = true);
		void GenerateCurve(int series);
	};
}

