#pragma once
#include "basictypes.h"
#include "View.h"
#include "Frame.h"
#include "RootView.h"
#include <vector>
#include "Menu.h"
#include <functional>

//#define _SHOWBORDER

namespace FlameUI
{
	extern ID3D11Device1* gD3D11Device;
	extern ID2D1Device1* gD2DDevice;
	extern IDXGIDevice1* gDXGIDevice;
	extern ID2D1Factory2* gD2DFactory;
	extern IDWriteFactory* gDWFactory;
	extern IDCompositionDevice* gDCompDevice;

	extern IWICImagingFactory2* gWICFactory;
	extern vector<RootView*> gRootViews;
	extern HANDLE gRenderThread;
	extern CRITICAL_SECTION gThreadAccess;
	extern bool disposed;
	extern D2D1_POINT_2F dpiScaleFactor;
	extern wchar_t locale[85];
	extern std::string ws2s(std::wstring wstr);
	extern std::wstring s2ws(std::string wstr);
	extern ID2D1DeviceContext1* gDeviceContext;

	namespace Debug
	{
		extern bool showBorder;
		extern View* tempView;
	}

#ifdef _DX12
	extern ID3D11DeviceContext* gD3D11Context;
	extern ID3D11On12Device* gD3D11On12Device;
	extern ID3D12Device4* gD3D12Device;


#endif

	namespace Theme
	{
		extern float BasicTextSize;
		extern float LineHeight;
		extern float LinePadding;
		extern float BorderWidth;
		namespace Color
		{
			extern D2D1_COLOR_F Front;
			extern D2D1_COLOR_F Background;
			extern D2D1_COLOR_F Theme;

		}
	}

	class MenuFrame;
	class DockProvider;
	typedef struct
	{
		animation_id id;
		animation_callback callback;
		WPARAM wParam;
		LPARAM lParam;
		time_t beginTime;
		time_t time;
		void* reserved;
	}AnimationData;


	IDXGIFactory3* GetFactory(IDXGIDevice* device);
	int AdaptColor(int color, float delta = 0.2f, int offset = 0);
	D2D1_COLOR_F AdaptColor(D2D1_COLOR_F color, float delta = 0.2f, int offset = 0);
	bool PointInRect(D2D1_POINT_2F pt, D2D1_RECT_F r);

	void Initiate();
	LRESULT SendEvent(View* hView, Message msg, WPARAM wParam, LPARAM lParam);
	void PostEvent(View* hView, Message msg, WPARAM wParam, LPARAM lParam);
	View* Create(View* parent, wchar_t* clsName, wchar_t name, LayoutData& layout, void* initData = nullptr);
	animation_id Animate(animation_callback callback, time_t duration, WPARAM wParam, LPARAM lParam, animation_id id_override = -1);
	animation_id Animate(View* target, time_t duration, WPARAM wParam, LPARAM lParam, animation_id id_override = -1);
	timer_id RegisterTimer(std::function<void()> cb,time_t duration);
	void AddRootView(RootView* rf);
	MenuFrame* _popMenu(Frame* root, Menu* menu, MenuCallback cb, void* thiz, POINT manual);

	template <typename T>
	MenuFrame* PopupMenu(Frame* root, Menu* menu, T callback, void* thiz = 0, POINT manual = { -158,-155 })
	{
		return _popMenu(root, menu, MakeCallback<T, MenuCallback>(callback), thiz, manual);
	}

	time_t GetTime();

	D2D1_SIZE_F MeasureTextSize(const wchar_t* str, D2D1_SIZE_F border, IDWriteTextFormat* format);
	void MainThreadDispatch();
	void _addDockProvider(DockProvider* dp);
	void _removeDockProvider(DockProvider* dp);
	void _rootDragDispatch(DockProvider* dp, Point mousePos, Message msg);

	void RunInMainThread(std::function<void(void)> func);
	void RunInUIThread(std::function<void(void)> func);
	FlameUI::Rect CalcViewRectOnScreen(View* v);
	template <typename T>
	inline void SafeRelease(T*& pI)
	{
		if (NULL != pI)
		{
			pI->Release();
			pI = NULL;
		}
	}
	void ThemeUpdate();

	ID2D1Bitmap* LoadBitmapPath(wstring path);
}



namespace Easings
{
	float Liner(float t);
	float QuintIn(float t);
	float QuintOut(float t);
	float ExpoOut(float t);
}