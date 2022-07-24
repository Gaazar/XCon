#pragma once
#include "View.h"
#include "Menu.h"
namespace FlameUI
{
	class MenuBar :public View
	{
	private:
		Menu* menu = nullptr;
		std::vector<float> tws;
		int hvrid = -1;
		int opid = -1;
		std::function<void(Menu*, int)> callback;

	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
	public:
		MenuBar(View* parent);
		void SetMenu(Menu* m);
		void Callback(std::function<void(Menu*, int)> cb);
	};
}
