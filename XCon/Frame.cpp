#include "Frame.h"
#include <string>
#include <stdexcept>
#include <system_error>
#include <memory>
#include <iostream>
#include "shellscalingapi.h"
#include "windows.h"
#include "FlameUI.h"
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "dcomp")

using namespace FlameUI;
using namespace D2D1;
#pragma region util

enum class Style : DWORD {
	windowed = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
	aero_borderless = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
	basic_borderless = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
	noanim_borderless = WS_SYSMENU
};

struct hwnd_deleter {
	using pointer = HWND;
	auto operator()(HWND handle) const -> void {
		::DestroyWindow(handle);
	}
}; using unique_handle = std::unique_ptr<HWND, hwnd_deleter>;

auto maximized(HWND hwnd) -> bool {
	WINDOWPLACEMENT placement;
	if (!::GetWindowPlacement(hwnd, &placement)) {
		return false;
	}

	return placement.showCmd == SW_MAXIMIZE;
}

auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void {
	if (!maximized(window)) {
		return;
	}

	auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
	if (!monitor) {
		return;
	}

	MONITORINFO monitor_info{};
	monitor_info.cbSize = sizeof(monitor_info);
	if (!::GetMonitorInfoW(monitor, &monitor_info)) {
		return;
	}

	// when maximized, make the client area fill just the monitor (without task bar) rect,
	// not the whole window rect which extends beyond the monitor.
	rect = monitor_info.rcWork;
}

auto Frame::hit_test(POINT cursor) const -> LRESULT {
	// identify borders and corners to allow resizing the window.
	// Note: On Windows 10, windows behave differently and
	// allow resizing outside the visible window frame.
	// This implementation does not replicate that behavior.
	const POINT border{
		::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
		::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
	};
	RECT window;
	if (!::GetWindowRect(hWnd, &window)) {
		return HTNOWHERE;
	}

	const auto drag = flags.drag ? HTCAPTION : HTCLIENT;

	enum region_mask {
		client = 0b0000,
		left = 0b0001,
		right = 0b0010,
		top = 0b0100,
		bottom = 0b1000,
	};

	const auto result =
		left * (cursor.x < (window.left + border.x)) |
		right * (cursor.x >= (window.right - border.x)) |
		top * (cursor.y < (window.top + border.y)) |
		bottom * (cursor.y >= (window.bottom - border.y));

	switch (result) {
	case left: return sizable ? HTLEFT : drag;
	case right: return sizable ? HTRIGHT : drag;
	case top: return sizable ? HTTOP : drag;
	case bottom: return sizable ? HTBOTTOM : drag;
	case top | left: return sizable ? HTTOPLEFT : drag;
	case top | right: return sizable ? HTTOPRIGHT : drag;
	case bottom | left: return sizable ? HTBOTTOMLEFT : drag;
	case bottom | right: return sizable ? HTBOTTOMRIGHT : drag;
	case client: return drag;
	default: return HTNOWHERE;
	}
}
auto last_error(const std::string& message) -> std::system_error {
	return std::system_error(
		std::error_code(::GetLastError(), std::system_category()),
		message
	);
}

auto composition_enabled() -> bool {
	BOOL composition_enabled = FALSE;
	bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
	return composition_enabled && success;
}
auto window_class(WNDPROC wndproc) -> const wchar_t* {
	static const wchar_t* window_class_name = [&] {
		WNDCLASSEXW wcx{};
		wcx.cbSize = sizeof(wcx);
		wcx.style = CS_HREDRAW | CS_VREDRAW;
		wcx.hInstance = nullptr;
		wcx.lpfnWndProc = wndproc;
		wcx.lpszClassName = L"Frame";
		wcx.hbrBackground = CreateSolidBrush(0x00212121);
		wcx.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
		const ATOM result = ::RegisterClassExW(&wcx);
		if (!result) {
			throw last_error("failed to register window class");
		}
		return wcx.lpszClassName;
	}();
	return window_class_name;
}

auto create_window(HWND parent, WNDPROC wndproc, void* userdata, SIZE size, POINT posi, DWORD extStyles = 0) -> HWND {
	auto handle = CreateWindowExW(
		extStyles, window_class(wndproc), L"Frame",
		static_cast<DWORD>(Style::aero_borderless), posi.x, posi.y,
		size.cx, size.cy, parent, nullptr, nullptr, userdata
	);
	if (!handle) {
		throw last_error("failed to create window");
	}
	return handle;
}

auto set_shadow(HWND handle, bool enabled) -> void {
	if (composition_enabled()) {
		static const MARGINS shadow_state[2]{ { 0,0,0,0 },{ 1,1,1,1 } };
		::DwmExtendFrameIntoClientArea(handle, &shadow_state[enabled]);
	}
}
auto select_borderless_style() -> Style {
	return composition_enabled() ? Style::aero_borderless : Style::basic_borderless;
}

void Frame::set_borderless(bool enabled) {
	Style new_style = (enabled) ? select_borderless_style() : Style::windowed;
	Style old_style = static_cast<Style>(::GetWindowLongPtrW(hWnd, GWL_STYLE));

	if (new_style != old_style) {

		::SetWindowLongPtrW(hWnd, GWL_STYLE, static_cast<LONG>(new_style));

		// when switching between borderless and windowed, restore appropriate shadow state
		set_shadow(hWnd, (new_style != Style::windowed));

		// redraw frame
		::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		::ShowWindow(hWnd, SW_SHOW);
	}
}

void Frame::set_borderless_shadow(bool enabled) {
	set_shadow(hWnd, enabled);
}
#pragma endregion

bool Frame::first = 1;

void TME(HWND hWnd)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof tme;
	tme.dwFlags = TME_HOVER | TME_LEAVE;
	tme.hwndTrack = hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);
}
LRESULT Frame::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT ret = 0;

	if (msg == WM_NCCREATE) {
		auto userdata = reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams;
		// store window instance pointer in window user data
		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
	}
	if (auto window_ptr = reinterpret_cast<Frame*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)))
	{
		auto& frame = *window_ptr;
		switch (msg)
		{
		case WM_CREATE:
			frame.OnCreate();
			//std::cout << "WMC\n";
			break;
		case WM_NCCALCSIZE:
		{
			auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
			adjust_maximized_client_rect(hwnd, params.rgrc[0]);
			return 0;
		}
		case WM_NCHITTEST:
			// When we have no border or title bar, we need to perform our
			// own hit testing to allow resizing and moving.
			return frame.hit_test(POINT{
				GET_X_LPARAM(lparam),
				GET_Y_LPARAM(lparam)
				});
		case WM_NCACTIVATE:
			if (!composition_enabled()) {
				// Prevents window frame reappearing on window activation
				// in "basic" theme, where no aero shadow is present.
				return 1;
			}
			break;
		case WM_PAINT:
			frame.Render();
			break;
		case WM_SIZE:
			if (frame.dcompTarget)
				frame.Resize();
			break;
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		{
			//EnterCriticalSection(&gThreadAccess);
			if (!frame.trackm)
			{
				//TME(hwnd);
				frame.trackm = true;
			}
			float ofx = frame.size.width;
			RECT wr;
			GetWindowRect(frame.hWnd, &wr);
			D2D1_POINT_2F mouse{ (GET_X_LPARAM(lparam) - wr.left) / dpiScaleFactor.x,(GET_Y_LPARAM(lparam) - wr.top) / dpiScaleFactor.y };
			if (msg == WM_MOUSEMOVE || frame.flags.maximized)
			{
				mouse.x += wr.left / dpiScaleFactor.x;
				mouse.y += wr.top / dpiScaleFactor.y;
			}
			if (mouse.y != -1)
			{
				frame.mousepos.x = mouse.x;
				frame.mousepos.y = mouse.y;
			}
			//std::cout << msg << ":" << mouse.x << "," << mouse.y << "," << wr.left << "," << wr.top << std::endl;
			bool v, r = false;
			if (frame.close)
			{
				ofx -= 46;
				v = PointInRect(mouse, { ofx,0,ofx + 46,30 });
				if (frame.flags.close != v)
				{
					frame.render.dirty = true;
				}
				frame.flags.close = v;
				r |= v;
			}
			if (frame.maximal)
			{
				ofx -= 46;
				v = PointInRect(mouse, { ofx,0,ofx + 46,30 });
				if (frame.flags.max != v)
				{
					frame.render.dirty = true;
				}
				frame.flags.max = v;
				r |= v;
			}
			if (frame.minimal)
			{
				ofx -= 46;
				v = PointInRect(mouse, { ofx,0,ofx + 46,30 });
				if (frame.flags.min != v)
				{
					frame.render.dirty = true;
				}
				frame.flags.min = v;
				r |= v;
			}
			frame.flags.drag = !r;
			if (!r)
			{
				LRESULT ret = 0;
				//std::cout << "mouse:" << mouse.x << "," << mouse.y << " foco:" << frame.mouseDown.foco << std::endl;
				if (frame.mouseDown.foco)
				{
					auto p = frame.mouseFoci->Position(true);
					mouse.x -= p.x;
					mouse.y -= p.y;
					frame.mouseDown.foco->SendEvent(FE_MOUSEMOVE, (WPARAM)&mouse, 0xffff0fff);
				}
				else
					ret = frame.SendEvent(FE_MOUSEMOVE, (WPARAM)&mouse, 0);
				frame.flags.drag = frame.dragble && (ret == (LRESULT)&frame);

				//std::cout << "MouseFoco:" << ret << std::endl;
			}
			if (frame.flags.nclbd && msg == WM_NCMOUSEMOVE)
			{
				WndProc(hwnd, WM_LBUTTONUP, wparam, frame.mousepos.x | (frame.mousepos.y << 16));
				frame.flags.nclbd = false;
			}
			//std::cout << "drag:" << frame.tState.drag << std::endl;
			//LeaveCriticalSection(&gThreadAccess);
			break;
		}
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		{
			WndProc(hwnd, WM_MOUSEMOVE, wparam, lparam);
			float ofx = frame.size.width;
			D2D1_POINT_2F mouse{ GET_X_LPARAM(lparam) / dpiScaleFactor.x,GET_Y_LPARAM(lparam) / dpiScaleFactor.y };
			if (frame.close)
			{
				ofx -= 46;
				if (PointInRect(mouse, { ofx,0,ofx + 46,30 }))
				{
					if (msg == WM_LBUTTONUP)
						PostMessageW(hwnd, WM_CLOSE, 0, 0);
					break;
				}
			}
			if (frame.maximal)
			{
				ofx -= 46;
				if (PointInRect(mouse, { ofx,0,ofx + 46,30 }))
				{
					if (msg == WM_LBUTTONUP)
					{
						if (frame.flags.maximized)
							PostMessageW(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
						else
							PostMessageW(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
					}
					break;
				}
			}
			if (frame.minimal)
			{
				ofx -= 46;
				if (PointInRect(mouse, { ofx,0,ofx + 46,30 }))
				{
					if (msg == WM_LBUTTONUP)
						PostMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					break;
				}
			}
		}
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			//case WM_SETCURSOR:
		{
			WndProc(hwnd, WM_MOUSEMOVE, wparam, lparam);
			//EnterCriticalSection(&gThreadAccess);
			D2D1_POINT_2F mouse{ GET_X_LPARAM(lparam) / dpiScaleFactor.x,GET_Y_LPARAM(lparam) / dpiScaleFactor.y };
			POINT cm;
			cm.x = mouse.x * dpiScaleFactor.x;
			cm.y = mouse.y * dpiScaleFactor.y;
			LRESULT ret = 0;
			//std::cout << "mouse:" << mouse.x << "," << mouse.y << std::endl;
			if (frame.mouseFoci)
			{
				auto p = frame.mouseFoci->Position(true);
				mouse.x -= p.x;
				mouse.y -= p.y;
				if (msg == FE_LBUTTONDOWN) { frame.mouseDown.mb = 0; frame.mouseDown.foco = frame.mouseFoci; if (!frame.flags.drag) SetCapture(hwnd); }
				if (msg == FE_RBUTTONDOWN) { frame.mouseDown.mb = 1; frame.mouseDown.foco = frame.mouseFoci; if (!frame.flags.drag) SetCapture(hwnd); }
				if (msg == FE_MBUTTONDOWN) { frame.mouseDown.mb = 2; frame.mouseDown.foco = frame.mouseFoci; if (!frame.flags.drag) SetCapture(hwnd); }
				if (msg == FE_LBUTTONUP && frame.mouseDown.mb == 0) { frame.mouseDown.foco = nullptr; ReleaseCapture(); PostMessageW(hwnd, WM_MOUSEMOVE, 0, cm.x | (cm.y << 16)); }
				if (msg == FE_RBUTTONUP && frame.mouseDown.mb == 1) { frame.mouseDown.foco = nullptr; ReleaseCapture(); PostMessageW(hwnd, WM_MOUSEMOVE, 0, cm.x | (cm.y << 16)); }
				if (msg == FE_MBUTTONUP && frame.mouseDown.mb == 2) { frame.mouseDown.foco = nullptr; ReleaseCapture(); PostMessageW(hwnd, WM_MOUSEMOVE, 0, cm.x | (cm.y << 16)); }
				ret = frame.mouseFoci->SendEvent(msg, wparam, ((int)mouse.x) | ((int)mouse.y) << 16);
				if (frame.mouseFoci && frame.mouseFoci->keyable)
				{
					frame.SetKeyboardFoci(frame.mouseFoci);
				}

			}
			//LeaveCriticalSection(&gThreadAccess);
			if (ret == 0)
			{
				break;
			}
			return ret;
		}
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		{
			RECT wr;
			GetWindowRect(frame.hWnd, &wr);
			POINT mouse{ (GET_X_LPARAM(lparam) - wr.left) ,(GET_Y_LPARAM(lparam) - wr.top) };
			WndProc(hwnd, msg + 0x160, wparam, mouse.x | (mouse.y << 16));
			if (msg == WM_NCLBUTTONDOWN) frame.flags.nclbd = true;
			break;
		}
		case WM_SYSCOMMAND:
		{
			if ((wparam & SC_MAXIMIZE) == SC_MAXIMIZE || (wparam & SC_RESTORE) == SC_RESTORE)
			{
				PostMessageW(hwnd, WM_EXITSIZEMOVE, 0, 0);
				if ((wparam & SC_MAXIMIZE) == SC_MAXIMIZE)
				{
					frame.flags.maximized = true;
					frame.View::Position();
				}
				if ((wparam & SC_RESTORE) == SC_RESTORE)
				{
					frame.flags.maximized = false;
					frame.View::Position();
				}
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (frame.mouseFoci)
			{
				//EnterCriticalSection(&gThreadAccess);
				frame.mouseFoci->SendEvent(msg, wparam, lparam);
				//LeaveCriticalSection(&gThreadAccess);
			}
			//frame.UpdateAll();
			break;
		}
		case WM_CHAR:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_IME_COMPOSITION:
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_ENDCOMPOSITION:
			if (frame.keyboadrFoci)
			{
				//EnterCriticalSection(&gThreadAccess);
				ret = frame.keyboadrFoci->SendEvent(msg, wparam, lparam);

				//LeaveCriticalSection(&gThreadAccess);
			}
			if (msg == WM_KEYDOWN && wparam == 9/*k_tab*/ && ret == 0)
			{
				frame.MoveFoci();
				//OutputDebugString(L"move foci\n");
			}
			break;
		case WM_DESTROY:
		{
			if (!frame.deleted)
			{
				EnterCriticalSection(&gThreadAccess);
				frame.SendEvent(FE_DESTROY, 0, 0);
				frame.deleted = true;
				frame.rootContext->Release();
				frame.swapChain->Release();
				frame.rootContext = 0;

				LeaveCriticalSection(&gThreadAccess);
			}
			if (&frame == gRootViews[0])
			{
				PostQuitMessage(0);
			}
			break;
		}
		case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT& mi = *(PMEASUREITEMSTRUCT)lparam;
			mi.itemHeight = 32;
			mi.itemWidth = 280;

			return 1;
		}
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT& ds = *(PDRAWITEMSTRUCT)lparam;

			FillRect(ds.hDC, &ds.rcItem, (HBRUSH)COLOR_DESKTOP);
			return 1;
		}
		case WM_KILLFOCUS:
		{
			frame.mouseDown.foco = 0;
			//frame.SetKeyboardFoci(0);
			frame.SetMouseFoci(0);
			frame.SendEvent(msg, wparam, lparam);
			break;
		}
		case WM_MOUSELEAVE:
		{
			frame.trackm = false;
			//std::cout << "Leave\n";
			break;
		}
		case WM_SETCURSOR:
		{
			if (frame.mouseFoci)
			{
				auto ret = frame.mouseFoci->SendEvent(msg, wparam, lparam);
				if (ret != 0)
					return ret;
			}
			break;
		}
		case WM_MOVE:
		case WM_EXITSIZEMOVE:
			int x = GET_X_LPARAM(lparam) + frame.mousepos.x;
			int y = GET_Y_LPARAM(lparam) + frame.mousepos.y;
			if (frame.mainDockProvider)
			{
				_rootDragDispatch(frame.mainDockProvider, { (float)x,(float)y }, msg);
				//std::cout << "move:" << x << "," << y << "|" << frame.mousepos.x << "," << frame.mousepos.y << std::endl;
			}
			break;

		}

	}
	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void Frame::DispatchEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	D2D1_POINT_2F& mouse = *(D2D1_POINT_2F*)wParam;
	switch (msg)
	{
	case FE_MOUSEMOVE:
	{

	}

	default:
		break;
	}
}


void D2DCreateContextFromHWND(HWND hwnd, IDXGISwapChain1** swapChain, ID2D1DeviceContext1** context)
{
	HRESULT hr;
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	RECT r;
	GetClientRect(hwnd, &r);
	scDesc.Width = r.right - r.left;
	scDesc.Height = r.bottom - r.top;
	scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	//scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	//scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//scDesc.Scaling = DXGI_SCALING_STRETCH;
	//scDesc.Stereo = true;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	auto dxgiF = GetFactory(gDXGIDevice);

	hr = dxgiF->CreateSwapChainForComposition(gDXGIDevice, &scDesc, nullptr, swapChain);
	hr = gD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, context);
	D2D1_BITMAP_PROPERTIES1 bp = BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	IDXGISurface* surface;
	(*swapChain)->GetBuffer(0, IID_PPV_ARGS(&surface));
	/*bp.dpiX = GetDeviceCaps(GetDC(0), 88);
	bp.dpiY = GetDeviceCaps(GetDC(0), 90);
	//gD2DFactory->GetDesktopDpi(&bp.dpiX, &bp.dpiY);
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_TARGET;*/

	ID2D1Bitmap1* bmp;
	hr = (*context)->CreateBitmapFromDxgiSurface(surface, bp, &bmp);
	if (hr != S_OK)
	{
		bmp = nullptr;
	}
	(*context)->SetTarget(bmp);
	bmp->Release();
	surface->Release();

}

Frame::Frame(SIZE wndSize, POINT wndPosition, DWORD extStyles, HWND parent) :dragble(true), sizable(true), rootContext(nullptr), swapChain(nullptr)
{
	mainDockProvider = nullptr;
	parenthWnd = parent;
	SIZE scaledsz{ wndSize.cx * dpiScaleFactor.x ,wndSize.cy * dpiScaleFactor.x };
	render.direct = true;
	hWnd = create_window(parent, Frame::WndProc, this, scaledsz, wndPosition, extStyles | WS_EX_NOREDIRECTIONBITMAP);
	//TME(hWnd);
	D2DCreateContextFromHWND(hWnd, &swapChain, &rootContext);
	rootContext->SetDpi(96 * dpiScaleFactor.x, 96 * dpiScaleFactor.y);
	//gRootViews.push_back(this);
	render.dirty = true;
	size.width = wndSize.cx;
	size.height = wndSize.cy;
	rect.right = size.width;
	rect.bottom = size.height;
	close = true;
	maximal = true;
	minimal = true;
	titled = true;
	D2D1_BITMAP_PROPERTIES1 bp = BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET,
		PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	gD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &render.context);
	auto hr = render.context->CreateBitmap(D2D1::SizeU(scaledsz.cx, scaledsz.cy), nullptr, 0, bp, &render.content);
	if (hr != S_OK)std::cout << "Error create content bitmap failed." << hr << std::endl;
	render.context->SetTarget(render.content);
	root = this;
	hImc = ImmAssociateContext(hWnd, 0);
	gDWFactory->CreateTextFormat(L"",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Theme::BasicTextSize,
		locale,
		&font
	);
	dTextFormat = font;
	if (first)
	{
		gDeviceContext = render.context;
	}
	//IDCompositionDevice3* dcd3;
	//hr = gDCompDevice->QueryInterface(&dcd3);
	//IDCompositionGaussianBlurEffect* dcgbe;
	//dcd3->CreateGaussianBlurEffect(&dcgbe);
	hr = gDCompDevice->CreateTargetForHwnd(hWnd, true, &dcompTarget);
	hr = gDCompDevice->CreateVisual(&dcompVisual);
	hr = dcompVisual->SetContent(swapChain);
	hr = dcompTarget->SetRoot(dcompVisual);
	//dcompVisual->SetEffect(dcgbe);
	//gDCompDevice->CreateEff
	//dcompVisual->SetEffect();
	hr = gDCompDevice->Commit();
	//keyable = true;



}
Frame::Frame(Frame* parent, SIZE wndSize, POINT wndPosition, DWORD extStyles) :Frame(wndSize, wndPosition, extStyles, parent->GetNative())
{
	this->parent = parent;
}
void Frame::Close()
{
	PostMessageW(hWnd, WM_CLOSE, 0, 0);
}
void Frame::Show()
{
	set_borderless(true);
	set_borderless_shadow(true);
	ShowWindow(hWnd, SW_SHOW);
	AddRootView(this);

}

void Frame::MainLoop(bool modeled)
{

	if (first)
	{
		ResumeThread(gRenderThread);
		first = 0;
	}
	else
	{
		isModel = modeled;
	}
	MSG msg;
	HWND phWnd = 0;
	if (modeled && parent)
	{
		phWnd = ((Frame*)parent)->GetNative();
		EnableWindow(phWnd, false);
	}
	while (::GetMessageW(&msg, 0, 0, 0) == TRUE && !FlameUI::disposed)
	{
		MainThreadDispatch();
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}
	if (modeled && phWnd)
		EnableWindow(phWnd, true), SetForegroundWindow(phWnd);;
}

#include <cstdlib>
//#include "C:\Users\Tibyl\OneDrive\SourceCode\GxEngine\GxEngine.h"


bool Frame::Render()
{

	EnterCriticalSection(&gThreadAccess);
	render.dirty = true;
	if (rootContext)
	{

		rootContext->BeginDraw();
		rootContext->Clear();
		SendEvent(FE_PAINT, (WPARAM)&localTransform, (LPARAM)rootContext);
		//ID2D1SolidColorBrush* br;

		//rootContext->CreateSolidColorBrush(D2D1::ColorF::ColorF(ColorF::Red), &br);
		//POINT pt{ rand() % 1000 / 1000.0f * 800, rand() % 1000 / 1000.0f * 600 };
		//rootContext->FillRectangle(D2D1::RectF(pt.x, pt.y, pt.x + 25, pt.y + 25), br);
		//br->Release();
		//rootContext->DrawBitmap((ID2D1Bitmap*)c2dbmp, D2D1::RectF(50, 50, 250, 250));
		auto hr = rootContext->EndDraw();



#ifdef _DX12

		gD3D11Context->Flush();
		//ReleaseClipLayer();

#endif

		swapChain->Present(1, 0);

		//IDXGISwapChain3* sc3;
		//hr = swapChain->QueryInterface(&sc3);
		//auto idx = sc3->GetCurrentBackBufferIndex();

		//render.direct = false;
#ifdef _DX12
		//ReleaseClipLayer();
#endif

	}
	render.dirty = false;
	LeaveCriticalSection(&gThreadAccess);
	return false;
}

void Frame::Resize()
{
	RECT r;
	GetClientRect(hWnd, &r);
	EnterCriticalSection(&gThreadAccess);
	D2D1_BITMAP_PROPERTIES1 bp = BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
	rootContext->SetTarget(nullptr);
	if (swapChain->ResizeBuffers(2, r.right - r.left, r.bottom - r.top, DXGI_FORMAT_B8G8R8A8_UNORM, 0) != S_OK)
	{
		std::cout << "Resize Window Failed." << std::endl;
		return;
	}
	ID2D1Bitmap1* bmp;
	IDXGISurface* surface;
	swapChain->GetBuffer(0, IID_PPV_ARGS(&surface));
	rootContext->CreateBitmapFromDxgiSurface(surface, bp, &bmp);
	rootContext->SetTarget(bmp);
	render.dirty = true;
	rootContext->SetDpi(96 * dpiScaleFactor.x, 96 * dpiScaleFactor.y);
	size = bmp->GetSize();
	size.width /= dpiScaleFactor.x;
	size.height /= dpiScaleFactor.y;
	View::Size(size);
	bmp->Release();
	surface->Release();
	auto hr = dcompVisual->SetContent(swapChain);
	hr = gDCompDevice->Commit();

	//SendEvent(FE_SIZED, (WPARAM)&size, 0);
	LeaveCriticalSection(&gThreadAccess);

}

void Frame::Draw()
{
	auto ctx = BeginDraw(Theme::Color::Background);
	ID2D1SolidColorBrush* closeRed;
	ID2D1SolidColorBrush* front;
	ID2D1SolidColorBrush* back;
	ID2D1SolidColorBrush* adapted;

	ctx->CreateSolidColorBrush(ColorF::ColorF(0x00E81123), &closeRed);
	ctx->CreateSolidColorBrush(Theme::Color::Front, &front);
	ctx->CreateSolidColorBrush(Theme::Color::Background, &back);
	ctx->CreateSolidColorBrush(AdaptColor(Theme::Color::Background, 0.1f), &adapted);

	float ofx = size.width;
	if (close)
	{
		ofx -= 46;
		if (flags.close)
		{
			ctx->FillRectangle({ ofx,0,ofx + 46,30 }, closeRed);
			ctx->DrawLine({ ofx + 20.5f,11.5f }, { ofx + 29.5f,20.5f }, back);
			ctx->DrawLine({ ofx + 20.5f,20.5f }, { ofx + 29.5f,11.5f }, back);
		}
		else
		{
			ctx->DrawLine({ ofx + 20.5f,11.5f }, { ofx + 29.5f,20.5f }, front);
			ctx->DrawLine({ ofx + 20.5f,20.5f }, { ofx + 29.5f,11.5f }, front);
		}
	}
	if (maximal)
	{
		ofx -= 46;
		if (flags.max)
		{
			ctx->FillRectangle({ ofx,0,ofx + 46,30 }, adapted);
		}
		if (flags.maximized)
		{
			ofx += 18;
			ctx->DrawRectangle({ ofx + 0.5f,14.5f , ofx + 7.5f, 21.5f }, front, 1);
			ctx->DrawLine({ ofx + 2.5f,12.5f }, { ofx + 2.5f, 14.5f }, front, 1);
			ctx->DrawLine({ ofx + 2.f,12.5f }, { ofx + 10.f, 12.5f }, front, 1);
			ctx->DrawLine({ ofx + 9.5f,20.5f }, { ofx + 9.5f, 12.5f }, front, 1);
			ctx->DrawLine({ ofx + 8.f,20.5f }, { ofx + 10.f, 20.5f }, front, 1);
			ofx -= 18;
		}
		else
			ctx->DrawRectangle({ ofx + 19.5f,11.5f,ofx + 28.5f,20.5f }, front);
	}
	if (minimal)
	{
		ofx -= 46;
		if (flags.min)
		{
			ctx->FillRectangle({ ofx,0,ofx + 46,30 }, adapted);
		}
		ctx->DrawLine({ ofx + 19.5f,15.5f }, { ofx + 28.5f, 15.5 }, front);
	}
	if (titled)
	{
		ctx->DrawTextW(title.c_str(), title.length(), font, { 10,8,size.width,32 }, front, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
	}
	closeRed->Release();
	front->Release();
	back->Release();
	adapted->Release();
	EndDraw();
}

void Frame::Position(POINT p)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	MoveWindow(hWnd, p.x, p.y, rc.right - rc.left, rc.bottom - rc.top, 0);
}
void Frame::Size(SIZE sz)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	MoveWindow(hWnd, rc.left, rc.top, sz.cx, sz.cy, 1);
	PostMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);
}
POINT Frame::Position()
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	return { rc.left,rc.top };
}
SIZE Frame::Size()
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	return { rc.right - rc.left,rc.bottom - rc.top };
}

POINT Frame::GetCursorPos()
{
	return { mousepos.x,mousepos.y };
}

wstring Frame::Title()
{
	return title;
}
void Frame::Title(wstring t)
{
	title = t;
	SetWindowTextW(hWnd, title.c_str());
}
