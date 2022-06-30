#include "TextEditBase.h"
#include "FlameUI.h"
#include "Frame.h"
#include "Winuser.h"
#include <iostream>

#pragma comment(lib,"Imm32.lib")
#define CLAMP(v,i,x) ( ((v)>=(i)) && ((v)<=(x)) ? v : ((v) < (i)) ? (i) : (x))

using namespace D2D1;
using namespace FlameUI;

LRESULT TextEditBase::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_GETKEYBOARD:
	{
		Frame& frame = *(Frame*)root;
		if (!flags.readonly && !flags.password)
		{
			ImmAssociateContext(frame.GetNative(), frame.hImc);
		}
		states.focusing = true;
		Animate(1000, 0, 0, animation.id);
		break;
	}
	case FE_LOSTKEYBOARD:
	{
		Frame& frame = *(Frame*)root;
		if (!flags.readonly && !flags.password)
		{
			frame.hImc = ImmAssociateContext(frame.GetNative(), 0);
		}
		states.focusing = false;
		break;
	}
	case WM_IME_COMPOSITION:
	{
		Frame& frame = *(Frame*)root;
		auto hImc = ImmGetContext(frame.GetNative());
		COMPOSITIONFORM cf = {};
		cf.dwStyle = CFS_RECT | CFS_FORCE_POSITION;
		DWRITE_HIT_TEST_METRICS metr;
		FLOAT ptx, pty;
		textLayout->HitTestTextPosition(selection.from, false, &ptx, &pty, &metr);
		cf.ptCurrentPos.x = (Position(true).x + metr.left + metr.width) * dpiScaleFactor.x;
		cf.ptCurrentPos.y = (Position(true).y + metr.top + metr.height) * dpiScaleFactor.y - 18;
		cf.rcArea.left = cf.ptCurrentPos.x;
		cf.rcArea.left = cf.ptCurrentPos.y;
		//cf.rcArea.right = cf.rcArea.left + 250;
		//cf.rcArea.bottom = cf.rcArea.top + 22;

		ImmSetCompositionWindow(hImc, &cf);
		if ((lParam & GCS_RESULTSTR) != 0)
		{
			std::wstring wstr;
			wstr.resize(ImmGetCompositionStringW(hImc, GCS_RESULTSTR, 0, 0) / 2);
			ImmGetCompositionStringW(hImc, GCS_RESULTSTR, &wstr[0], wstr.length() * 2);
			Animate(1000, 0, 0, animation.id);
			content.erase(selection.from - selection.complen, (UINT32)selection.complen);
			selection.to = selection.from -= selection.complen;
			Paste(wstr);
			Refresh();
			textLayout->SetUnderline(false, { 0, (UINT32)wstr.length() });
			selection.complen = 0;

		}
		if ((lParam & GCS_COMPSTR) != 0)
		{
			if (selection.from != selection.to) Backspace();
			std::wstring wstr;
			wstr.resize(ImmGetCompositionStringW(hImc, GCS_COMPSTR, 0, 0) / 2);
			ImmGetCompositionStringW(hImc, GCS_COMPSTR, &wstr[0], wstr.length() * 2);
			Animate(1000, 0, 0, animation.id);
			content.erase(selection.from - selection.complen, (UINT32)selection.complen);
			selection.to = selection.from -= selection.complen;
			Paste(wstr);
			Refresh();
			selection.complen = wstr.length();
			//Content(wstr.c_str());
			textLayout->SetUnderline(true, { selection.from - (UINT32)wstr.length(), (UINT32)wstr.length() });
		}

		ImmReleaseContext(frame.GetNative(), hImc);
		break;
	}
	case FE_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_BACK:
			Backspace();
			break;
		case VK_CONTROL:
			states.ctrl = true;
			break;
		case VK_SHIFT:
			states.shift = true;
			break;
		case VK_LEFT:
		case VK_RIGHT:
		{
			if (states.shift)
			{
				if (wParam == VK_LEFT)
					selection.to--;
				else
					selection.to++;
				MakeHightlight();
			}
			else
			{
				if (wParam == VK_LEFT)
					selection.from = --selection.to;
				else
					selection.from = ++selection.to;
			}
			Animate(1000, 0, 0, animation.id);
			selection.to = CLAMP(selection.to, 0, content.length());
			break;
		}
		case 65://a
		{
			if (states.ctrl)
			{
				selection.from = 0;
				selection.to = content.length();
				MakeHightlight();
			}
			break;
		}
		case 67://c
		{
			if (states.ctrl)
			{
				Copy();
			}
			break;
		}
		case 86://v
		{
			if (states.ctrl)
			{
				Paste();
			}
			break;
		}
		case 88://x
		{
			if (states.ctrl)
			{
				if (selection.from != selection.to)
				{
					Copy();
					Backspace();
				}
			}
			break;
		}
		case 9://tab
		{
			if (flags.tabable)
				return 1;
			break;
		}
		}

		break;
	}
	case FE_KEYUP:
	{

		switch (wParam)
		{
		case VK_CONTROL:
			states.ctrl = false;
			break;
		case VK_SHIFT:
			states.shift = false;
			break;
		}
		break;
	}
	case FE_CHAR:
	{
		if (states.ctrl)
		{
		}
		else
			if (!states.compositing && wParam != 8 && !flags.readonly && lParam != 1)
			{
				if (!flags.multiline && (wParam == '\r' || wParam == '\n'))
					break;
				if (!flags.tabable && wParam == '\t')
					break;
				if (selection.from != selection.to) Backspace();
				content.insert(selection.from, 1, (const wchar_t)(wParam));

				selection.to = ++selection.from;
				Animate(1000, 0, 0, animation.id);
				Refresh();
			}
		break;
	}
	case FE_LBUTTONDOWN:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		BOOL isTrailing, inside;
		DWRITE_HIT_TEST_METRICS metr;
		textLayout->HitTestPoint(x, y, &isTrailing, &inside, &metr);
		selection.from = metr.textPosition;
		if (isTrailing) selection.from++;
		if ((content[selection.from] & 0xFC00) == 0xDC00 || content[selection.from] == 0x200D) selection.from++;
		selection.to = selection.from;
		//std::cout << isTrailing << inside << metr.textPosition << std::endl;
		states.pressed = true;
		if (states.compositing)
		{
			Frame& frame = *(Frame*)root;
			if (!flags.readonly && !flags.password)
			{
				frame.hImc = ImmAssociateContext(frame.GetNative(), 0);
				ImmAssociateContext(frame.GetNative(), frame.hImc);
			}
		}
		break;
	}
	case FE_LBUTTONUP:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		BOOL isTrailing, inside;
		DWRITE_HIT_TEST_METRICS metr;
		textLayout->HitTestPoint(x, y, &isTrailing, &inside, &metr);
		selection.to = metr.textPosition;
		if (isTrailing) selection.to++;
		if ((content[selection.to] & 0xFC00) == 0xDC00 || content[selection.to] == 0x200D) selection.to++;
		states.pressed = false;
		MakeHightlight();
		break;
	}
	case FE_MOUSEMOVE:
	{
		if (states.pressed)
		{
			float x = GET_X_LPARAM(lParam);
			float y = GET_Y_LPARAM(lParam);
			BOOL isTrailing, inside;
			DWRITE_HIT_TEST_METRICS metr;
			textLayout->HitTestPoint(x, y, &isTrailing, &inside, &metr);
			selection.to = metr.textPosition;
			if (isTrailing) selection.to++;
			if ((content[selection.to] & 0xFC00) == 0xDC00 || content[selection.to] == 0x200D) selection.to++;
			MakeHightlight();
		}
		break;
	}
	case WM_IME_STARTCOMPOSITION:
		states.compositing = true;
		break;
	case WM_IME_ENDCOMPOSITION:
		states.compositing = false;
		break;
	case FE_RBUTTONDOWN:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		MenuItem mi;
		if (selection.geometry)
		{
			BOOL ct = 0;
			selection.geometry->FillContainsPoint({ x,y }, nullptr, &ct);
			if (!ct)
			{
				OnEvent(FE_LBUTTONDOWN, wParam, lParam);
				OnEvent(FE_LBUTTONUP, wParam, lParam);
				texteditMenu->GetItem(0, mi);
				mi.disabled = true;
				texteditMenu->ModifyItem(0, mi);
				texteditMenu->GetItem(2, mi);
				mi.disabled = true;
				texteditMenu->ModifyItem(2, mi);
				texteditMenu->GetItem(5, mi);
				mi.disabled = true;
				texteditMenu->ModifyItem(5, mi);
			}
		}
		break;
	}
	case FE_RBUTTONUP:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		MenuItem mi;
		if (selection.geometry)
		{
			BOOL ct = 0;
			selection.geometry->FillContainsPoint({ x,y }, nullptr, &ct);
			if (ct)
			{
				texteditMenu->GetItem(0, mi);
				mi.disabled = false;
				texteditMenu->ModifyItem(0, mi);
				texteditMenu->GetItem(2, mi);
				mi.disabled = false;
				texteditMenu->ModifyItem(2, mi);
				texteditMenu->GetItem(5, mi);
				mi.disabled = false;
				texteditMenu->ModifyItem(5, mi);
			}
		}
		PopupMenu((Frame*)root, texteditMenu, &TextEditBase::MenuCallback, this);
		break;
	}
	case WM_SETCURSOR:
	{
		auto hr = SetCursor(LoadCursor(nullptr, IDC_IBEAM));
		return 1;
	}
	default:
		break;
	}

	return 0;
}

void TextEditBase::MakeGeom()
{
	UINT32 f, t;
	f = min(selection.from, selection.to);
	t = max(selection.from, selection.to);
	textLayout->SetUnderline(false, { 0,(UINT32)content.length() });
	textLayout->SetUnderline(true, { f,t - f });
	UpdateView();
}

void TextEditBase::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* br;
	FLOAT px, py;
	DWRITE_HIT_TEST_METRICS metr;
	if (textLayout)
	{
		ctx->CreateSolidColorBrush(Theme::Color::Theme, &br);
		if (selection.geometry && selection.from != selection.to)
		{
			ctx->FillGeometry(selection.geometry, br);
		}
		br->SetColor(Theme::Color::Front);
		ctx->DrawTextLayout({ 0,0 }, textLayout, br, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		auto hr = textLayout->HitTestTextPosition(selection.to, false, &px, &py, &metr);
		if (hr != S_OK)
			cout << "error hit test" << endl;
		//br->SetColor(ColorF::ColorF(Theme::Color::Front.r, Theme::Color::Front.g, Theme::Color::Front.b, animation.opacity));
		br->SetOpacity(animation.opacity);
		ctx->DrawLine({ metr.left,metr.top }, { metr.left,metr.top + metr.height }, br, 1.5f);
		br->Release();
	}

	EndDraw();
}
void TextEditBase::Animation(float progress, int p1, int p2)
{
	if (p1 == -1) return;
	if (progress == 1 && states.focusing)
		Animate(1000, 0, 0, animation.id);
	//cout << "animate:" << progress<< endl;
	if (!states.focusing)
	{
		animation.opacity = 0;
		UpdateView();
	}
	else
	{
		if (progress < 0.3f)
		{
			animation.opacity = 1;
		}
		if (progress > 0.3f && progress < 0.5f)
		{
			animation.opacity = 1 - (progress - 0.3f) / 0.2f;
		}
		if (progress > 0.8f)
		{
			animation.opacity = (progress - 0.8f) / 0.2f;
		}
		if (animation.opacity > 0 && animation.opacity <= 1)
			UpdateView();
		animation.opacity = CLAMP(animation.opacity, 0.0f, 1.0f);
	}
}
TextEditBase::TextEditBase(View* parent) :View(parent)
{
	texteditMenu = new Menu(L"TextEditMenu");
	texteditMenu->AppendItem(MenuItem::Common(0, 0, L"复制", L"Ctrl+C"));
	texteditMenu->AppendItem(MenuItem::Common(1, 0, L"粘贴", L"Ctrl+V"));
	texteditMenu->AppendItem(MenuItem::Common(2, 0, L"剪切", L"Ctrl+X"));
	texteditMenu->AppendItem(MenuItem::Seperator());
	texteditMenu->AppendItem(MenuItem::Common(3, 0, L"全选", L"Ctrl+A"));
	texteditMenu->AppendItem(MenuItem::Common(4, 0, L"删除", L"Backspace"));
	texteditMenu->AppendItem(MenuItem::Common(5, 0, L"撤销", L"Ctrl+Z", true));

	textLayout = nullptr;
	selection.geometry = nullptr;
	keyable = true;
	flags.multiline = false;
	flags.tabable = false;
	flags.password = false;
	gDWFactory->CreateTextFormat(L"", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, Theme::BasicTextSize, locale, &textFormat);
	animation.id = Animate(0, -1, 0);
	Content(L"");
}
void TextEditBase::Content(wstring c)
{
	content = c;
	Refresh();
}
wstring TextEditBase::Content()
{
	return content;
}
void TextEditBase::Refresh()
{
	if (textLayout) textLayout->Release();
	if (layout.sizeMode.x == SIZE_MODE_CONTENT || layout.sizeMode.y == SIZE_MODE_CONTENT)
	{

	}

	if (flags.password)
	{
		wchar_t* pwd = new wchar_t[content.length() + 1]{ 0 };
		for (int i = 0; i < content.length(); ++i)
		{
			pwd[i] = L'●';
		}
		gDWFactory->CreateTextLayout(pwd, content.length(), textFormat, rect.width(), rect.height(), &textLayout);
		delete pwd;
	}
	else
		gDWFactory->CreateTextLayout(content.c_str(), content.length(), textFormat, rect.width(), rect.height(), &textLayout);
	MakeHightlight();
	UpdateView();

}

const wchar_t* TextEditBase::Copy()
{
	if (selection.from != selection.to && !flags.password)
	{
		if (OpenClipboard(0))
		{
			auto f = min(selection.from, selection.to);
			auto t = max(selection.from, selection.to);
			EmptyClipboard();
			auto str = content.substr(f, t - f);
			auto hTxt = GlobalAlloc(GMEM_ZEROINIT, str.length() * 2 + 2);
			GlobalLock(hTxt);
			memcpy(hTxt, str.c_str(), str.length() * 2 + 2);
			GlobalUnlock(hTxt);
			SetClipboardData(CF_UNICODETEXT, hTxt);
			CloseClipboard();
		}
	}
	return nullptr;
}
void TextEditBase::Paste(wstring txt)
{
	if (selection.from != selection.to)
		Backspace();
	content.insert(selection.from, txt);
	selection.to = selection.from += txt.length();

}
void TextEditBase::Paste()
{
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (OpenClipboard(0))
		{
			auto c = GetClipboardData(CF_UNICODETEXT);
			auto l = GlobalLock(c);
			wstring cstr((const wchar_t*)l);
			Paste(cstr);
			GlobalUnlock(l);
			Refresh();
			CloseClipboard();
		}
	}

}

void TextEditBase::Backspace()
{
	if (flags.readonly) return;
	if (selection.from != selection.to)
	{
		auto f = min(selection.from, selection.to);
		auto t = max(selection.from, selection.to);
		content.erase(f, t - f);
		selection.from = selection.to = f;
	}
	else
	{
		if (selection.from > 0)
		{
			if ((content[selection.from - 1] & 0xFC00) == 0xDC00)
			{
				content.erase(selection.from - 2, 2);
				selection.to = selection.from -= 2;
			}
			else
			{
				content.erase(selection.from - 1, 1);
				selection.to = --selection.from;
			}

		}
	}
	Refresh();
}

void TextEditBase::MakeHightlight()
{
	EnterCriticalSection(&gThreadAccess);
	if (selection.geometry)
		selection.geometry->Release();
	auto hr = gD2DFactory->CreatePathGeometry(&selection.geometry);
	if (hr != S_OK)
		return;
	ID2D1GeometrySink* gs;

	auto f = min(selection.from, selection.to);
	auto t = max(selection.from, selection.to);
	if (f != t)
	{
		//cout << f << " to " << t << " ," << t - f << endl;
		selection.geometry->Open(&gs);
		FLOAT px, py;
		DWRITE_HIT_TEST_METRICS metr;
		float ll = -999, tt = -999;
		for (int i = f; i < t; i++)
		{
			//cout << (int)content[f] << endl;
			if (content[i] == 0x200D)
			{
				i += 2;
				continue;
			}
			textLayout->HitTestTextPosition(i, false, &px, &py, &metr);
			//if (ll == metr.left && tt == metr.top) continue;
			ll = metr.left; tt = metr.top;
			gs->BeginFigure({ metr.left,metr.top }, D2D1_FIGURE_BEGIN_FILLED);
			gs->AddLine({ metr.left + metr.width,metr.top });
			gs->AddLine({ metr.left + metr.width,metr.top + metr.height });
			gs->AddLine({ metr.left ,metr.top + metr.height });
			gs->EndFigure(D2D1_FIGURE_END_CLOSED);
			if ((content[i] & 0xFC00) == 0xD800)
			{
				i++;
				continue;
			}
		}
		gs->Close();
		gs->Release();
	}
	UpdateView();
	LeaveCriticalSection(&gThreadAccess);
}

void TextEditBase::Multiline(bool v)
{
	flags.multiline = v;
}
void TextEditBase::Readonly(bool v)
{
	flags.readonly = v;
}
void TextEditBase::Password(bool v)
{
	flags.password = v;
}
void TextEditBase::Tabable(bool v)
{
	flags.tabable = v;
}

void TextEditBase::MenuCallback(Menu* m, int id)
{
	switch (id)
	{
	case 0:
		Copy();
		break;
	case 1:
		Paste();
		break;
	case 2:
		if (selection.from != selection.to)
		{
			Copy();
			Backspace();
		}
		break;
	case 3:
		selection.from = 0;
		selection.to = content.length();
		MakeHightlight();
		break;
	case 4:
		Backspace();
		break;
	default:
		break;
	}
}
