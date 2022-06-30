#pragma once
#include "View.h"
#include "IDockProvider.h"
#include <string>
#include "Panel.h"
#include "basictypes.h"
#include <vector>
#include "SeperatorHandle.h"
namespace FlameUI
{
	enum SIDE
	{
		SIDE_LEFT, SIDE_TOP, SIDE_RIGHT, SIDE_BOTTOM, SIDE_CENTER
	};
	class DockProvider :
		public View/*, public IDockProvider*/
	{
		struct Table
		{
			std::wstring name;
			Panel* content;
			float length;
		};
		struct DockInfo
		{
			bool isDiv;
			View* div0;
			View* div1;
			SeperatorHandle* seph;
			DockInfo(bool isDivision, View* d0, View* d1 = nullptr, SeperatorHandle* sh = nullptr)
			{
				this->isDiv = isDivision;
				this->div0 = d0; this->div1 = d1; this->seph = sh;
			}
		};
		int current = 0;
		int miIdx = -1;
		int press = -1;
	private:
		~DockProvider();
		Point pressOffset;
		std::vector<Table> tabs;
		Point mpos;
		SIDE placeSide;
		Panel* previewer;
	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void AddTab(wstring name, Panel* panel);
	public:
		bool docking = false;
		DockProvider(View* parent);
		Panel* AddTab(wstring name);
		void PreviewBegin();
		void PreviewUpdate(Point cursor);
		void PreviewEnd(bool place, Table tab);
		Table GetPlaceTab();
		void ClearTabs();
		//void Preview(View* content, Point mousePos) override;
		//void Place(View* content, Point mousePos) override;

	};
}
