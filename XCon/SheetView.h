#pragma once
#include "View.h"
#include "Scroller.h"
#include <string>
namespace FlameUI
{
#define SVE_SELECT 0x1FF0
	using namespace std;
	typedef struct tagTitle
	{
		const wstring display;
		float weight;

		float p, w;
	} Title;
	struct Item
	{
		wstring disp;
		void* val;
	};
	class SheetView :
		public View
	{
		Scroller* scrl;
		vector<Title> titles;
		vector<vector<Item>> data;
		float totalWeight = 0;
		IDWriteTextFormat* fmt_ttl;
		IDWriteTextFormat* fmt_cnt;
		int selected = -1;
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;

	public:
		float heightCol = 30;
		SheetView(View* parent);
		SheetView* AddTitle(wstring display, float weight = 1);
		void AddRow(initializer_list<Item> data);
		void Modify(int row, initializer_list<Item> data);
		void Modify(int row, int col, Item data);
		void Clear();
		int Selected() { return selected; }
		void Selected(int s) { selected = s; UpdateView(); };
		Item* Selection(int row, int col);
	};
}
