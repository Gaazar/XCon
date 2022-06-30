#pragma once
#include "basictypes.h"
#include "RootView.h"
#include <string>
#include "Windows.h"
#include "windowsx.h"
#include "dwmapi.h"
#include "dcomp.h"

#define FS_DRAG 1
#define FS_SIZEBOX 2

namespace FlameUI
{

	enum FrameStyle
	{
		FrameStyle_Resize,
		FrameStyle_Minimum,
		FrameStyle_Maximum,
		FrameStyle_Titled,
		FrameStyle_Dragable
	};

	class Frame :
		public RootView
	{
	private:
		static bool first;
		HWND parenthWnd;
		HWND hWnd;
		ID2D1DeviceContext1* rootContext;
		IDXGISwapChain1* swapChain;
		IDWriteTextFormat* font;
		IDCompositionTarget* dcompTarget;
		IDCompositionVisual* dcompVisual;
		bool isModel;
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		auto hit_test(POINT cursor) const->LRESULT;
		auto set_borderless(bool enabled) -> void;
		auto set_borderless_shadow(bool enabled) -> void;
		void Resize();
		void Draw() override;
		struct
		{
			bool close;
			bool max;
			bool min;
			bool mdown;
			bool drag;
			bool nclbd;
			bool maximized;
		} flags;
		wstring title = L"Frame";

	public:
		HIMC hImc;
		bool trackm;
		bool dragble;
		bool sizable;
		bool maximal;
		bool minimal;
		bool close;
		bool titled;
		POINT mousepos;
		Frame(SIZE wndSize, POINT wndPosition = { CW_USEDEFAULT,CW_USEDEFAULT }, DWORD extStyles = 0,HWND parent = 0);
		Frame(Frame* parent, SIZE wndSize, POINT wndPosition = { CW_USEDEFAULT,CW_USEDEFAULT }, DWORD extStyles = 0);
		//~Frame();
		HWND GetNative() { return hWnd; }
		void Show();
		void MainLoop(bool modeled = false);
		void Paint();
		bool Render() override;
		void DispatchEvent(Message msg, WPARAM wParam, LPARAM lParam);
		void Close();
		void Position(POINT);
		void Size(SIZE);
		POINT Position();
		SIZE Size();
		POINT GetCursorPos() override;
		wstring Title();
		void Title(wstring t);
		virtual void OnCreate() {};
	};
}