#pragma once
#include "vector"
#include <d2d1_3.h>
#include <dxgi.h>
#include <dxgi1_5.h>
#include <wincodec.h>
#include <windowsx.h>
#include "dwrite.h"
#include <d3d11_4.h>
#include "dcomp.h"
#include <string>
#include <functional>

using namespace std;
//#define _SHOWBORDER
//#define _DX12

#ifdef _DX12

#include <d3d11on12.h>
#include <d3d12.h>

#endif


namespace FlameUI {
#define FE_PAINT WM_PAINT
#define FE_SIZED WM_SIZE
#define FE_MOUSEMOVE		WM_MOUSEMOVE
#define FE_LBUTTONDOWN		WM_LBUTTONDOWN
#define FE_LBUTTONUP		WM_LBUTTONUP
#define FE_LBUTTONDBCLK		WM_LBUTTONDBLCLK
#define FE_LBUTTONCLICK		0x1A00
#define FE_RBUTTONDOWN		WM_RBUTTONDOWN
#define FE_RBUTTONUP		WM_RBUTTONUP
#define FE_LBUTTONDBCLK		WM_LBUTTONDBLCLK
#define FE_RBUTTONCLICK		0x1A01
#define FE_MBUTTONDOWN		WM_MBUTTONDOWN
#define FE_MBUTTONUP		WM_MBUTTONUP
#define FE_LBUTTONDBCLK		WM_LBUTTONDBLCLK
#define FE_MBUTTONCLICK		0x1A02
#define FE_MOUSELEAVE		0x1A10
#define FE_MOUSEENTER		0x1A11
#define FE_GETKEYBOARD		0x1A12
#define FE_LOSTKEYBOARD		0x1A13
#define FE_CHAR				WM_CHAR
#define FE_KEYDOWN			WM_KEYDOWN
#define FE_KEYUP			WM_KEYUP
#define FE_SCROLL			WM_MOUSEWHEEL
#define FE_ANIMATION		0x1B00
#define FE_MOVED			WM_MOVE
#define FE_RADIO_BROADCAST	0x1B01
#define FE_RADIO			0x1B02
#define FE_CHILD			0x1B03
#define FE_PARENTSIZED		0x1B04
#define FE_CHILDSIZED		0x1B05
#define FE_CHILDMOVED		0x1B06
#define FE_DESTROY			WM_DESTROY
#define FE_ONSHOW			0x1B07
#define FE_CHANGE			0x1B08
#define FE_PARENTCHANGE		0x1B09
#define FE_ROOTDRAGING		0x1B0A
#define FE_DOCK				0x1B0B
#define FE_MOUSEHIT			0x1B0C



#define FE_S_L_FROMROOT		0xB000
#define FE_S_L_FROMPARENT	0xB001
	typedef std::wstring String;
	typedef D2D1_VECTOR_3F Vector3;
	typedef D2D1_COLOR_F Color;
	typedef ID2D1DeviceContext1* RenderContext;
	typedef int Message;
	typedef int animation_id;
	typedef int timer_id;
	typedef void (*animation_callback)(float progress, WPARAM w, LPARAM l);
	class View;
	class Menu;
	typedef LRESULT(*Method)(void* thiz, View* view, Message msg, WPARAM wparam, LPARAM lparam);
	typedef LRESULT(*Function)(View* view, Message msg, WPARAM wparam, LPARAM lparam);
	typedef void(*MenuCallback)(void* thiz, Menu* menu, int id);
	typedef struct
	{
		Message event;
		void* object;
		std::function<void(Message, WPARAM, LPARAM)> func;
	}EventListener;
	template <typename T>
	Method MakeListenerCallback(T callback)
	{
		union
		{
			Method listener;
			T raw;
		} cvt;
		cvt.raw = callback;
		return cvt.listener;
	};
	template <typename T, typename Tret>
	Tret MakeCallback(T callback)
	{
		union
		{
			Tret listener;
			T raw;
		} cvt;
		cvt.raw = callback;
		return cvt.listener;
	};
	struct Vector2
	{
		float x;
		float y;
		operator D2D1_POINT_2F ()
		{
			return { x,y };
		}
		Vector2(const D2D1_POINT_2F& d)
		{
			x = d.x;
			y = d.y;
		}
		Vector2(const D2D1_SIZE_F& d)
		{
			x = d.width;
			y = d.height;
		}
		Vector2(float x, float y)
		{
			this->x = x;
			this->y = y;
		}
		Vector2()
		{
			x = 0;
			y = 0;
		}
		Vector2 operator +(const Vector2& v)
		{
			return { x + v.x,y + v.y };
		}
		float SqrDistance()
		{
			return x * x + y * y;
		}

	};
	typedef Vector2 Point;
	struct Size
	{
		float width;
		float height;
		Size(const D2D1_SIZE_F& sz)
		{
			width = sz.width;
			height = sz.height;
		}
		Size(float x, float y)
		{
			width = x;
			height = y;
		}
		Size()
		{
			width = height = 0;
		}
		operator D2D1_SIZE_F ()
		{
			return { width,height };
		}
	};

	typedef struct
	{
		ID2D1Factory2* factory;
		ID2D1Device1* device;
	} InitiateData;

	struct Rect
	{
		union
		{
			struct
			{
				float left;
				float top;
				float right;
				float bottom;
			};
			float value[4];
		};

		inline float width() const noexcept { return right - left; }
		inline float height() const noexcept { return bottom - top; }
		operator D2D1_RECT_F()
		{
			return { left,top,right,bottom };
		}

		Rect(D2D1_RECT_F& r)
		{
			left = r.left;
			top = r.top;
			right = r.right;
			bottom = r.bottom;
		}
		Rect(Vector2 min, Vector2 max)
		{
			left = min.x;
			top = min.y;
			right = max.x;
			bottom = max.y;
		}
		Rect(D2D1_SIZE_F min, D2D1_SIZE_F max)
		{
			left = min.width;
			top = min.height;
			right = max.width;
			bottom = max.height;
		}
		Rect() :left(0), top(0), right(0), bottom(0) {};
		Rect(float l, float t, float r, float b) :left(l), top(t), right(r), bottom(b) {};

	};

	using Vector4 = Rect;
	typedef struct
	{
		Rect padding;
		Rect border;
		Rect margin;
	}Box;

	class Placer;

	typedef struct
	{
		float alpha = 0.0f;
		bool dirty;
		ID2D1DeviceContext2* context;
		ID2D1Bitmap1* content;
		ID2D1Layer* clipLayer;
	} RenderData;

	enum SIZE_MODE
	{
		SIZE_MODE_NONE,
		SIZE_MODE_CONTENT,
		SIZE_MODE_CHILDREN
	};
	enum COORD
	{
		COORD_POSTIVE,
		COORD_NEGATIVE,
		COORD_FILL,
		COORD_CENTER,
		COORD_RESERVE
	};
	struct Coord
	{
		COORD x;
		COORD y;
	};
	typedef struct
	{
		Box box;
		Rect maxSize;
		Rect minSize;
		D2D1_POINT_2F anchor;
		bool ignore;
		bool dirty;
		struct
		{
			SIZE_MODE x = SIZE_MODE_NONE;
			SIZE_MODE y = SIZE_MODE_NONE;
		} sizeMode;
		Coord coord;
		Placer* layout;
	} LayoutData;

	class Type;

	class MemberInfo
	{
	public:
		wchar_t* GetName();
	};
	class FieldInfo :MemberInfo
	{
	public:
		Type* GetType();
	};
	class MethodInfo :MemberInfo
	{
	public:
		Type* GetReturnType();
		void* GetParameters();

	};

	class Type
	{
	private:
		wchar_t* GetName();

	};
}