#pragma once
#include "basictypes.h"
#include <string>
namespace FlameUI
{
	enum MenuItemType
	{
		MenuItemType_Common,
		MenuItemType_Seperator,
		MenuItemType_SubMenu
	};
	class Menu;
	struct MenuItem
	{
		MenuItemType type;
		ID2D1Bitmap* icon = nullptr;
		std::wstring title = L"";
		std::wstring description = L"";
		UINT64 id;
		Menu* subMenu = nullptr;
		bool disabled;
		bool checked;
		static MenuItem Common(UINT64 id, ID2D1Bitmap* icon, wstring title, wstring decsription, bool disabled = false);
		static MenuItem Seperator();
		static MenuItem SubMenu(UINT64 id, Menu* menu);
	};
	class Menu
	{
	public:
		vector<MenuItem> items;
		std::wstring name = L"";

		Menu(std::wstring menuName);
		void AppendItem(MenuItem mi);
		void GetItem(int index, MenuItem& mi);
		void ModifyItem(int index, MenuItem mi);
		void SetDisabled(int index, bool disabled);
		void SetTitle(int index, wstring s);
		void SetDescription(int index, wstring s);
		void SetSubmenu(int index, Menu* submenu);

	};

}