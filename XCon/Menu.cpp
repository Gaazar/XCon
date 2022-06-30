#include "Menu.h"

using namespace FlameUI;
using namespace D2D1;
using namespace std;

MenuItem MenuItem::Common(int id, ID2D1Bitmap* icon, wstring title, wstring description, bool disabled)
{
	return { MenuItemType_Common,icon,title,description,id,nullptr,disabled };
}
MenuItem MenuItem::Seperator()
{
	return { MenuItemType_Seperator,nullptr,L"",L"",0,nullptr };

}
MenuItem MenuItem::SubMenu(int id, Menu* menu)
{
	return { MenuItemType_SubMenu,nullptr,menu->name,L"",id,menu };

}

Menu::Menu(std::wstring menuName)
{
	name = menuName;
}
void Menu::AppendItem(MenuItem mi)
{
	items.push_back(mi);
}
void Menu::GetItem(int index, MenuItem& mi)
{
	if (index < 0 || index >= items.size())
		return;
	mi = items[index];
}
void Menu::ModifyItem(int index, MenuItem mi)
{
	if (index < 0 || index >= items.size())
		return;
	items[index] = mi;

}
void Menu::SetDisabled(int index, bool disabled)
{
	if (index < 0 || index >= items.size())
		return;
	items[index].disabled = disabled;
}
void Menu::SetTitle(int index, wstring s)
{
	if (index < 0 || index >= items.size())
		return;
	items[index].title = s;

}
void Menu::SetDescription(int index, wstring s)
{
	if (index < 0 || index >= items.size())
		return;
	items[index].description = s;

}
void Menu::SetSubmenu(int index, Menu* submenu)
{
	if (index < 0 || index >= items.size())
		return;
	items[index].subMenu = submenu;

}

