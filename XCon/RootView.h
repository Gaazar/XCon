#pragma once
#include "View.h"
#include "d2d1_2.h"

namespace FlameUI
{
	class DockProvider;
	class RootView :public View
	{
	private:
	public:
		View* keyboadrFoci;
		View* mouseFoci;
		struct
		{
			int mb;// 0=L 1=R 2=M
			View* foco;
		}mouseDown;
		ID2D1Geometry* dirtyRegion;
		DockProvider* mainDockProvider;

	public:
		IDWriteTextFormat* dTextFormat;
		bool deleted = false;
		virtual bool Render() = 0;
		View* MoveFoci();
		void SetMouseFoci(View* newFoci);
		void SetKeyboardFoci(View* newFoci);
		virtual POINT GetCursorPos() = 0;
	};
}