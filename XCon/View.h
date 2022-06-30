#pragma once
#include "basictypes.h"
#include <string>

namespace FlameUI
{
	class View;
	class RootView;
	class Placer
	{
	public:
		virtual void Layout(vector<View*>& children) {};
	};

	class View
	{
	private:
		D2D1_RECT_F place();
		D2D1_SIZE_F area();
		vector<EventListener*> listeners;
		bool disposed;
	protected:
		RootView* root;
		View* parent;
		vector<View*> children;
		RenderData render;
		LayoutData layout;
		D2D1_MATRIX_3X2_F localTransform;
		D2D1_MATRIX_3X2_F finalTransform;
		D2D1_POINT_2F position = { 0,0 };
		D2D1_SIZE_F scale = { 1,1 };
		float rotation;
		D2D1_SIZE_F size;
		Rect rect;
		ID2D1DeviceContext1* BeginDraw(const D2D1_COLOR_F& clear, bool dontClear = false);
		void EndDraw();
		void DrawStyles(ID2D1DeviceContext1* ctx = nullptr) {};
		virtual LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) { return 0; }
		virtual void Draw() { DrawStyles(); render.dirty = false; }
		virtual void Animation(float progress, int p1, int p2) {};
		int z = 0;

	public:
		wstring name;
		bool disabled;
		bool mouseable;
		bool keyable;
		bool masked;
		bool monitor = false;
		bool custommousetest = false;
		long long value = 0;

		View();
		View(View*);

		RootView* Root();
		View* Parent();
		void Parent(View*);
		float Alpha();
		void Alpha(float);
		Rect Margin();
		void Margin(Rect, bool update = true);
		Rect Padding();
		void Padding(Rect, bool update = true);
		void Anchor(D2D1_POINT_2F);
		D2D1_POINT_2F Anchor();
		void Scale(D2D1_SIZE_F);
		D2D1_SIZE_F Scale();


		void SizeMode(SIZE_MODE x, SIZE_MODE y);
		Rect GetRect();
		void Layouter(Placer*);
		//inline Rect Border();
		//inline void Border(Rect, bool update = true);
		void Size(D2D1_SIZE_F);
		void Position(D2D1_POINT_2F, bool update = true);
		void UpdateTransform();
		void UpdateView();
		void Coord(COORD x, COORD y);
		FlameUI::Coord Coord();
		animation_id Animate(time_t duration, int param1, int param2, int id_override = -1);
		D2D1_POINT_2F Position(bool absolute = false);
		D2D1_SIZE_F Size();
		void UpdateAll();
		template <typename T>
		HANDLE AddEventListener(void* object, T callback, Message msg)
		{
			EventListener* el = new EventListener();
			el->object = object;
			el->method = MakeListenerCallback(callback);
			el->event = msg;
			listeners.push_back(el);
			return el;
		}
		void RemoveEventListener(HANDLE handle)
		{
			for (auto i = listeners.begin(); i != listeners.end();)
			{
				if ((*i) == handle)
				{
					i = listeners.erase(i);
				}
				else
					++i;
			}
		}
		inline bool isDirty()
		{
			return render.dirty;
		}

		LRESULT SendEvent(Message msg, WPARAM wParam, LPARAM lParam);
		bool Disposed();

		//LRESULT TestCallback(View* view, Message msg, WPARAM wParam, LPARAM lParam);
		View* FindChildFoci(View* after, bool& contains);
		D2D1_MATRIX_3X2_F FinalTransform();
		bool RemoveChild(View* c);
		void AppendChild(View* c);
		bool Contains(View* v, bool reversive);
		int ZOrder();
		void ZOrder(int zo, bool update = true);
		void UpdateZ();

		void ResetRoot(RootView* r);
		void Destroy();

		vector<View*> GetChildren();

		bool operator<(const View& v);
		bool operator>(const View& v);
	};
}