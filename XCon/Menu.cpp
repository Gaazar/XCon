#include "Menu.h"
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <codecvt>
#include "FlameUI.h"
using namespace std;
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
vector<string>& split(const string& s, char delim, vector<string>& elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
vector<Menu*> Menu::Load(const wchar_t* fpath)
{
	vector<Menu*> menus;
	ifstream fs(fpath, ios::in);
	map<wstring, Menu*> pathToMenu;
	int gid = 1;
	if (fs.good())
	{
		string sline;
		while (getline(fs, sline, '\n'))
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			wstring line = converter.from_bytes(sline);
			auto p = line.find_last_of('/');
			wstring name = line.substr(p + 1);
			wstring path = line.substr(0, p);
			vector<wstring> ppart;
			size_t bpos = 0;
			Menu* parent = nullptr;
			while (bpos != string::npos)
			{
				bpos = path.find_first_of('/', bpos + 1);
				wstring ppath = path.substr(0, bpos);
				if (pathToMenu.count(ppath) == 0)
				{
					auto p = ppath.find_last_of('/');
					Menu* t = new Menu(ppath.substr(p + 1));
					pathToMenu[ppath] = t;
					if (parent)
					{
						parent->AppendItem(MenuItem::SubMenu(0, t));
					}
					else
					{
						menus.push_back(t);
					}
					parent = t;
				}
				else
				{
					parent = pathToMenu[ppath];
				}
				//cout << path.substr(0, bpos) << " pos=" << bpos << endl;
			}
			if (name == L"|")
			{
				parent->AppendItem(MenuItem::Seperator());
			}
			else if (name[0] == '.')
			{
				Menu* t = new Menu(name.substr(1));
				parent->AppendItem(MenuItem::SubMenu(0, t));
			}
			else
			{
				std::wstring scid = L"";
				bool f = false;
				std::wstring rname = L"";
				for (int i = 0; i < name.length(); i++)
				{
					if (name[i] == ',')
					{
						f = true;
						continue;
					}

					if (f)
						scid += name[i];
					else
						rname += name[i];
				}
				if (scid == L"")
				{
					parent->AppendItem(MenuItem::Common(gid, nullptr, name, L""));
					gid++;
				}
				else
				{
					parent->AppendItem(MenuItem::Common(stoi(scid), nullptr, rname, L""));
				}
			}
		}
		fs.close();
	}
	//cout << "Menu Load Done\n";

	return menus;
}
