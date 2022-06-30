#include "RootView.h"
#include <iostream>
#include "FlameUI.h"
using namespace FlameUI;

#include <string>

View* RootView::MoveFoci()
{
	View* candidate = nullptr;
	View* v = this;
	if (keyboadrFoci)
		v = keyboadrFoci;
	bool cts = false;
	candidate = v->FindChildFoci(nullptr, cts);
	if (!candidate)
	{
		cts = false;
		candidate = FindChildFoci(v, cts);
	}
	if (candidate)
		SetKeyboardFoci(candidate);
	else
		SetKeyboardFoci(nullptr);
	return nullptr;
}
void RootView::SetMouseFoci(View* newFoci)
{
	if (mouseFoci != newFoci)
	{
		//wstring d = L"Switch mouse foco: " + std::to_wstring((unsigned long)(mouseFoci)) + L"->" + std::to_wstring((unsigned long)newFoci) + L"\n";
		//std::wcout << d;
		//OutputDebugString(d.c_str());
		auto lf = mouseFoci;
		mouseFoci = newFoci;
		if (lf)
			lf->SendEvent(FE_MOUSELEAVE, (WPARAM)newFoci, 0);
		if (newFoci)
			newFoci->SendEvent(FE_MOUSEENTER, (WPARAM)lf, 0);
		if (Debug::showBorder)
		{
			if (lf) lf->UpdateView();
			if (newFoci) newFoci->UpdateView();
		}
	}

}
void RootView::SetKeyboardFoci(View* newFoci)
{
	if (keyboadrFoci != newFoci)
	{
		auto lf = keyboadrFoci;
		keyboadrFoci = newFoci;
		if (lf)
		{
			lf->SendEvent(FE_LOSTKEYBOARD, (WPARAM)newFoci, 0);
		}
		if (newFoci)
			newFoci->SendEvent(FE_GETKEYBOARD, (WPARAM)lf, 0);
	}

}
