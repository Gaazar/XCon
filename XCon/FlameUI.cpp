#include "FlameUI.h"
#include "d3d11.h"
#include "shellscalingapi.h"
#include <iostream>
#include "dwmapi.h"
#include "MenuFrame.h"
#include "MenuPainter.h"
#include "DockProvider.h"

#pragma comment(lib, "Shcore.lib")

#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "D3D11.lib")

using namespace FlameUI;

struct Event
{
	View* view;
	Message msg;
	WPARAM wparam;
	LPARAM lparam;
};

vector<Event> deferredEvents;
vector<AnimationData> animations;
animation_id aid_counter = 0;

ID3D11Device1* FlameUI::gD3D11Device = nullptr;
ID2D1Device2* FlameUI::gD2DDevice = nullptr;
IDXGIDevice1* FlameUI::gDXGIDevice = nullptr;
ID2D1Factory3* FlameUI::gD2DFactory = nullptr;
IDWriteFactory* FlameUI::gDWFactory = nullptr;
IWICImagingFactory2* FlameUI::gWICFactory = nullptr;
IDCompositionDevice* FlameUI::gDCompDevice = nullptr;
HANDLE FlameUI::gRenderThread = 0;
DWORD RenderThreadId = 0;
DWORD MainThreadId = 0;
CRITICAL_SECTION FlameUI::gThreadAccess;
bool FlameUI::disposed = false;
vector<RootView*> FlameUI::gRootViews;
D2D1_POINT_2F FlameUI::dpiScaleFactor;
wchar_t FlameUI::locale[85];
vector<RootView*> rvWaitList;
vector<DockProvider*> dockProviders;
ID2D1DeviceContext1* FlameUI::gDeviceContext = 0;


vector<std::function<void(void)>> uiThreadProcess;
vector<std::function<void(void)>> mainThreadProcess;

#ifdef _DX12

ID3D11DeviceContext* FlameUI::gD3D11Context = nullptr;
ID3D11On12Device* FlameUI::gD3D11On12Device = nullptr;
ID3D12Device4* FlameUI::gD3D12Device = nullptr;

ID3D12Fence* gFence;
ID3D12CommandQueue* gCommandQueue;
#endif

using namespace D2D1;

D2D1_COLOR_F Theme::Color::Front = ColorF::ColorF(ColorF::White);
D2D1_COLOR_F Theme::Color::Background = ColorF::ColorF(0x00212121);
D2D1_COLOR_F Theme::Color::Theme = ColorF::ColorF(ColorF::DeepSkyBlue);

//D2D1_COLOR_F Theme::Color::Front = ColorF::ColorF(ColorF::Black);
//D2D1_COLOR_F Theme::Color::Background = ColorF::ColorF(ColorF::White);
//D2D1_COLOR_F Theme::Color::Theme = ColorF::ColorF(ColorF::DeepSkyBlue);

bool Debug::showBorder = false;
View* Debug::tempView = nullptr;
//For complex apps
float Theme::BasicTextSize = 13.0f; //15
float Theme::LineHeight = 18.f;		//24
float Theme::LinePadding = 5.0f;	//6
float Theme::BorderWidth = 1.0f;	//2


//For common apps									
//float Theme::BasicTextSize = 15.0f; //15
//float Theme::LineHeight = 24.f;		//24
//float Theme::LinePadding = 6.0f;	//6
//float Theme::BorderWidth = 2.0f;	//2

IDXGIFactory3* FlameUI::GetFactory(IDXGIDevice* device)
{
	HRESULT hr;
	IDXGIAdapter* adapter;
	IDXGIFactory3* factory;
	hr = device->GetAdapter(&adapter);
	hr = adapter->GetParent(IID_PPV_ARGS(&factory));
	return factory;
}


void Animate()
{
	auto ct = GetTime();
	if (animations.size() != 0)
	{
		EnterCriticalSection(&gThreadAccess);
		for (auto i = animations.begin(); i != animations.end();)
		{
			float p = ((float)(ct - (*i).beginTime)) / (*i).time;
			if (p >= 0)
			{
				if (p > 1) p = 1;
				if ((*i).reserved)
				{
					if (!((View*)(*i).reserved)->Disposed())
						((View*)(*i).reserved)->SendEvent(FE_ANIMATION, (WPARAM) & (*i), (LPARAM)&p);
				}
				else if (i->callback)
				{
					i->callback(p, i->wParam, i->lParam);
				}

			}
			if (ct >= (*i).beginTime + (*i).time)
			{
				i = animations.erase(i);

			}
			else ++i;
		}
		LeaveCriticalSection(&gThreadAccess);
	}
}

DWORD WINAPI RenderThread(LPVOID lpParam)
{
	while (!disposed)
	{
		bool dirty = false;
		if (uiThreadProcess.size() > 0)
		{
			EnterCriticalSection(&gThreadAccess);
			for (auto i : uiThreadProcess)
			{
				i();
			}
			uiThreadProcess.clear();
			LeaveCriticalSection(&gThreadAccess);
		}
		Animate();
		if (rvWaitList.size() != 0)
		{
			for (vector<RootView*>::iterator i = rvWaitList.begin(); i != rvWaitList.end(); ++i)
			{
				gRootViews.push_back(*i);
			}
			rvWaitList.clear();
		}
		//EnterCriticalSection(&gThreadAccess);
		for (vector<RootView*>::iterator i = gRootViews.begin(); i != gRootViews.end();)
		{
			if (!(*i)->deleted)
			{
				if ((*i)->isDirty())
				{
					(*i)->Render();
					dirty = true;
				}
				++i;
			}
			else
			{
				int ix = 0;
				i = gRootViews.erase(i);
			}
		}
		if (gRootViews.size() == 0)
		{
			disposed = true;
		}
		//LeaveCriticalSection(&gThreadAccess);
		if (!dirty)
			Sleep(16);
		else
			gDCompDevice->Commit();
	}
	return 0;
}

inline int clamp(int val, int min = 0, int max = 255)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}
inline float clamp(float val, float min = 0, float max = 1)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

int FlameUI::AdaptColor(int color, float delta, int offset)
{
	int r = (color & 0xFF0000) >> 16,
		g = (color & 0x00FF00) >> 8,
		b = color & 0x0000FF;
	float brightness = 0.299 * r + 0.587 * g + 0.114 * b;
	float mul;
	if (brightness > 128)
	{
		offset = -offset;
		mul = 1 - delta;
	}
	else
	{
		mul = 1 + delta;
	}
	r = clamp(r * mul + offset);
	g = clamp(g * mul + offset);
	b = clamp(b * mul + offset);

	return  (r << 16) | (g << 8) | (b);
}

D2D1_COLOR_F FlameUI::AdaptColor(D2D1_COLOR_F c, float delta, int offset)
{
	float brightness = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
	float mul;
	if (brightness > 0.5f)
	{
		offset = -offset;
		mul = 1 - delta;
	}
	else
	{
		mul = 1 + delta;
	}
	c.r = clamp(c.r * mul + offset);
	c.g = clamp(c.g * mul + offset);
	c.b = clamp(c.b * mul + offset);

	return  c;
}

bool FlameUI::PointInRect(D2D1_POINT_2F pt, D2D1_RECT_F r)
{
	if (pt.x < r.left) return false;
	if (pt.y < r.top) return false;
	if (pt.x > r.right) return false;
	if (pt.y > r.bottom) return false;
	return true;
}

#ifdef _DX12
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <dxgi1_6.h>

#include "C:\Users\Tibyl\OneDrive\SourceCode\GxEngine\d3dx12.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
using namespace Microsoft::WRL;

void GetHardwareAdapterA(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter = false)
{
	*ppAdapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;

	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}
	else
	{
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter.Detach();
}

#endif


void FlameUI::Initiate()
{
	HRESULT hr;
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

	GetUserDefaultLocaleName(locale, sizeof(locale));
	MainThreadId = GetCurrentThreadId();

	UINT dxgiFactoryFlags = 0;
	ID3D11Device* d11dev;

#ifdef _DX12
#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif


	ComPtr<IDXGIFactory5> factory;
	hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
	bool m_useWarpDevice = false;
	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

		hr = (D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&gD3D12Device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapterA(factory.Get(), &hardwareAdapter);

		hr = (D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&gD3D12Device)
		));
	}

	gD3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gFence));

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//UNORM是归一化处理的无符号整数
	msaaQualityLevels.SampleCount = 1;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 0;
	//当前图形驱动对MSAA多重采样的支持（注意：第二个参数即是输入又是输出）
	hr = (gD3D12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels)));
	//NumQualityLevels在Check函数里会进行设置
	//如果支持MSAA，则Check函数返回的NumQualityLevels > 0
	//expression为假（即为0），则终止程序运行，并打印一条出错信息
	assert(msaaQualityLevels.NumQualityLevels > 0);

	//Create Command Objects
	{
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		//hr = gD3D12Device->QueryInterface(&gDXGIDevice);

		hr = (gD3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&gCommandQueue)));
		//ThrowIfFailed(gD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&graphicsCommandAllocator)));
		//ThrowIfFailed(gD3D12Device->CreateCommandList(0, //掩码值为0，单GPU
		//	D3D12_COMMAND_LIST_TYPE_DIRECT, //命令列表类型
		//	graphicsCommandAllocator.Get(),	//命令分配器接口指针
		//	nullptr,	//流水线状态对象PSO，这里不绘制，所以空指针
		//	IID_PPV_ARGS(&graphicsCommandList)));	//返回创建的命令列表
		//ThrowIfFailed(graphicsCommandList->Close());


		D3D_FEATURE_LEVEL rlvl;

		hr = (D3D11On12CreateDevice(gD3D12Device, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
			reinterpret_cast<IUnknown**>(&gCommandQueue), 1, 0, &d11dev, &gD3D11Context, &rlvl));
		hr = d11dev->QueryInterface(&gD3D11Device);
		hr = (gD3D11Device->QueryInterface(&gD3D11On12Device));
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		hr = (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&gD2DFactory)));
		hr = (gD3D11On12Device->QueryInterface(&gDXGIDevice));
		hr = (gD2DFactory->CreateDevice(gDXGIDevice, &gD2DDevice));
		//ThrowIfFailed(gD2DDevice->CreateDeviceContext(deviceOptions, &d2dContext));
	}


#else
	D3D_FEATURE_LEVEL curLevel;
	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &d11dev, &curLevel, nullptr);
	hr = d11dev->QueryInterface(&gD3D11Device);

	gD3D11Device->QueryInterface(IID_PPV_ARGS(&gDXGIDevice));


	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, IID_PPV_ARGS(&gD2DFactory));
	hr = gD2DFactory->CreateDevice(gDXGIDevice, &gD2DDevice);
#endif

	gRenderThread = CreateThread(NULL, 0, RenderThread, 0, CREATE_SUSPENDED, &RenderThreadId);
	InitializeCriticalSection(&gThreadAccess);
	dpiScaleFactor.x = GetDeviceCaps(GetDC(0), 88) / 96.0f;
	dpiScaleFactor.y = GetDeviceCaps(GetDC(0), 90) / 96.0f;
	//dpiScaleFactor.x = 1.25f;
	//dpiScaleFactor.y = 1.25f;

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&gDWFactory));
	hr = DCompositionCreateDevice3(gDXGIDevice, IID_PPV_ARGS(&gDCompDevice));
	CoInitialize(0);
	hr = CoCreateInstance(CLSID_WICImagingFactory2, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&gWICFactory));

	DWORD color;
	BOOL blend;
	DwmGetColorizationColor(&color, &blend);
	Theme::Color::Theme = ColorF::ColorF(color);

}
void FlameUI::ThemeUpdate()
{
	RunInUIThread([]()
		{
			for (auto i : gRootViews)
			{
				i->UpdateAll();
			}
		});
}



View* FlameUI::Create(View* parent, wchar_t* clsName, wchar_t name, LayoutData& layout, void* initData)
{
	return nullptr;
}

animation_id FlameUI::Animate(animation_callback callback, time_t duration, WPARAM wParam, LPARAM lParam, animation_id id_override)
{
	animation_id current = id_override;
	//cout << "animation id override :" << id_override << endl;
	for (auto itr = animations.begin(); itr != animations.end(); ++itr)
	{
		//cout << "animation id :" << itr->id << endl;
		if (itr->id == id_override)
		{
			itr->beginTime = GetTime();
			itr->time = duration;
			itr->wParam = wParam;
			itr->lParam = lParam;
			return id_override;
		}
	}
	AnimationData data;
	if (current == -1)
		current = ++aid_counter;
	data.id = current;
	data.beginTime = GetTime();
	data.time = duration;
	data.reserved = nullptr;
	data.wParam = wParam;
	data.lParam = lParam;
	data.callback = callback;
	EnterCriticalSection(&gThreadAccess);
	animations.push_back(data);
	LeaveCriticalSection(&gThreadAccess);
	return current;
}

animation_id FlameUI::Animate(View* target, time_t duration, WPARAM wParam, LPARAM lParam, animation_id id_override)
{
	animation_id current = id_override;
	//cout << "animation id override :" << id_override << endl;
	for (auto itr = animations.begin(); itr != animations.end(); ++itr)
	{
		//cout << "animation id :" << itr->id << endl;
		if (itr->id == id_override)
		{
			itr->beginTime = GetTime();
			itr->time = duration;
			itr->wParam = wParam;
			itr->lParam = lParam;
			return id_override;
		}
	}
	AnimationData data;
	if (current == -1)
		current = ++aid_counter;
	data.id = current;
	data.beginTime = GetTime();
	data.time = duration;
	data.reserved = target;
	data.wParam = wParam;
	data.lParam = lParam;
	data.callback = nullptr;
	EnterCriticalSection(&gThreadAccess);
	animations.push_back(data);
	LeaveCriticalSection(&gThreadAccess);
	return current;
}

static LARGE_INTEGER startTime = { 0 };
time_t FlameUI::GetTime(double unit)
{
	LARGE_INTEGER m_liPerfFreq = { 0 };
	QueryPerformanceFrequency(&m_liPerfFreq);
	LARGE_INTEGER m_liPerfStart = { 0 };
	QueryPerformanceCounter(&m_liPerfStart);
	if (startTime.QuadPart == 0)
		startTime = m_liPerfStart;

	return (double)(m_liPerfStart.QuadPart - startTime.QuadPart) / m_liPerfFreq.QuadPart * unit;
}

void FlameUI::PostEvent(View* hView, Message msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == FE_PAINT || msg == FE_ANIMATION) return;
	EnterCriticalSection(&gThreadAccess);
	deferredEvents.push_back({ hView,msg,wParam,lParam });
	LeaveCriticalSection(&gThreadAccess);
}

void  FlameUI::AddRootView(RootView* rf)
{
	rvWaitList.push_back(rf);
}
MenuFrame* FlameUI::_popMenu(Frame* root, Menu* m, MenuCallback cb, void* thiz, POINT manual)
{
	//cout << "pop a menu" << endl;
	POINT wz, cp = root->GetCursorPos();
	wz = root->Position();
	wz.x += cp.x;
	wz.y += cp.y;
	if (manual.x >= 0 && manual.y >= 0)
	{
		wz = manual;
	}
	MenuFrame* mf = new MenuFrame(root, { wz.x  ,wz.y }, { 128,64 });
	MenuPainter* mp = new MenuPainter(mf, m, cb, thiz);
	mp->Position({ 0,1 });
	SIZE sz;
	sz.cx = mp->menuSize.width;
	sz.cy = mp->menuSize.height + 2;
	mf->Size(sz);
	mf->Show();
	return mf;
}
void FlameUI::MainThreadDispatch()
{
	if (mainThreadProcess.size() > 0)
	{
		EnterCriticalSection(&gThreadAccess);
		for (auto i : mainThreadProcess)
		{
			i();
		}
		mainThreadProcess.clear();
		LeaveCriticalSection(&gThreadAccess);
	}
	if (deferredEvents.size() != 0)
	{
		deferredEvents[0].view->SendEvent(deferredEvents[0].msg, deferredEvents[0].wparam, deferredEvents[0].lparam);
		deferredEvents.erase(deferredEvents.begin());
	}

}
D2D1_SIZE_F FlameUI::MeasureTextSize(const wchar_t* str, D2D1_SIZE_F border, IDWriteTextFormat* format)
{
	IDWriteTextLayout* layout;
	DWRITE_TEXT_METRICS m;
	auto hr = gDWFactory->CreateTextLayout(str, wcslen(str), format, border.width, border.height, &layout);
	layout->GetMetrics(&m);
	layout->Release();
	return { m.width,m.height };
}
void FlameUI::_addDockProvider(DockProvider* dp)
{
	dockProviders.push_back(dp);
}
void FlameUI::_removeDockProvider(DockProvider* dp)
{
	for (auto i = dockProviders.begin(); i != dockProviders.end(); i++)
	{
		if (*i == dp)
		{
			dockProviders.erase(i);
			return;
		}
	}
}
FlameUI::Rect FlameUI::CalcViewRectOnScreen(View* v)
{
	RECT rc;
	Frame* rootf = (Frame*)v->Root();
	GetWindowRect(rootf->GetNative(), &rc);
	auto t = v->FinalTransform();
	D2D1_POINT_2F pol = { 0,0 };
	auto por = pol * t;
	Point fpt = { rc.left + por.x, rc.top + por.y };
	//Point fpt = { rc.left , rc.top };
	return { fpt.x,fpt.y,fpt.x + v->GetRect().width(),fpt.y + v->GetRect().height() };
}
void FlameUI::_rootDragDispatch(DockProvider* rv, Point mousePos, Message msg)
{
	static DockProvider* currentPreview;
	bool empty = true;
	if (rv->docking)
	{
		for (auto i = dockProviders.begin(); i != dockProviders.end(); i++)
		{
			if (rv == *i) continue;
			auto rc = CalcViewRectOnScreen(*i);
			if (mousePos.x > rc.left && mousePos.x < rc.right && mousePos.y > rc.top && mousePos.y < rc.bottom)
			{
				if (currentPreview != *i)
				{
					if (currentPreview) currentPreview->PreviewEnd(false, {});
					currentPreview = *i;
					if (currentPreview) currentPreview->PreviewBegin();
				}
				if (currentPreview)
				{
					currentPreview->PreviewUpdate({ (mousePos.x - rc.left) / dpiScaleFactor.x,(mousePos.y - rc.top) / dpiScaleFactor.y });
				}
				empty = false;
				//wcout << L"Docking detected. " << rv->name << L" is preparing to dock with DockProvider:" << (*i)->name << endl;
				//wcout << L"RECT: " << rc.left << L", " << rc.top << L", " << rc.right << L", " << rc.bottom << endl;
				//wcout << L"MPOS: " << mousePos.x << L", " << mousePos.y << endl;
				break;
			}
		}
	}
	if (empty && currentPreview)
	{
		if (msg == WM_EXITSIZEMOVE)
		{
			auto c = currentPreview;
			RunInUIThread([c, rv]()
				{
					c->PreviewEnd(true, rv->GetPlaceTab());
					rv->ClearTabs();
					if (rv->Root()->mainDockProvider) rv->Root()->Destroy();
				});
		}
		else
		{
			currentPreview->PreviewEnd(false, {});

		}
		currentPreview = nullptr;
	}

}

void FlameUI::RunInMainThread(std::function<void(void)> func)
{
	if (GetCurrentThreadId() == MainThreadId)
	{
		func();
	}
	else
	{
		EnterCriticalSection(&gThreadAccess);
		mainThreadProcess.push_back(func);
		LeaveCriticalSection(&gThreadAccess);
	}

}
void FlameUI::RunInUIThread(std::function<void(void)> func)
{
	if (GetCurrentThreadId() == RenderThreadId)
	{
		func();
	}
	else
	{
		EnterCriticalSection(&gThreadAccess);
		uiThreadProcess.push_back(func);
		LeaveCriticalSection(&gThreadAccess);
	}
}

std::string FlameUI::ws2s(std::wstring wstr)
{
	// support chinese
	std::string res;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), nullptr, 0, nullptr, nullptr);
	if (len <= 0) {
		return res;
	}
	char* buffer = new char[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, nullptr, nullptr);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;
}

std::wstring FlameUI::s2ws(std::string wstr)
{
	std::wstring res;
	int len = MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), wstr.size(), nullptr, 0);
	if (len < 0) {
		return res;
	}
	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;
}

ID2D1Bitmap* FlameUI::LoadBitmapPath(wstring path)
{
	IWICBitmapDecoder* decoder;
	auto hr = gWICFactory->CreateDecoderFromFilename(path.c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	IWICBitmapFrameDecode* bdf;
	hr = decoder->GetFrame(0, &bdf);
	if (hr != S_OK) return nullptr;
	IWICFormatConverter* cvt;
	gWICFactory->CreateFormatConverter(&cvt);
	hr = cvt->Initialize(bdf, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom);
	if (hr != S_OK) return nullptr;
	IWICBitmap* wbmp;
	hr = gWICFactory->CreateBitmapFromSource(cvt, WICBitmapNoCache, &wbmp);
	if (hr != S_OK) return nullptr;
	ID2D1Bitmap* bmp;
	hr = gDeviceContext->CreateBitmapFromWicBitmap(wbmp, &bmp);
	if (hr != S_OK) return nullptr;
	bdf->Release();
	decoder->Release();
	wbmp->Release();
	return bmp;

}





float Easings::Liner(float t) { return t; }
float Easings::QuintIn(float t)
{
	return t * t * t * t * t;
}
float Easings::QuintOut(float t)
{
	t = t - 1;
	return t * t * t * t * t + 1;
}
float Easings::ExpoOut(float t)
{
	if (t == 0 || t == 1)
		return t;
	return 1 - powf(2, -10 * t);

}




