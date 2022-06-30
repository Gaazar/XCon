#include "View.h"
#include "FlameUI.h"
#include <math.h>
#include <iostream>
#include <algorithm>
//#include <typeinfo>
using namespace FlameUI;
using namespace D2D1;

View::View() :parent(nullptr), disabled(false), keyable(false), mouseable(true), localTransform()
{
	//name = s2ws(typeid(this).raw_name());
	name = L"New Object";
	render.grouped = false;
	render.dirty = true;
	render.direct = false;
	masked = true;
	localTransform = D2D1::Matrix3x2F::Identity();
	finalTransform = localTransform;
	layout.coord = { COORD_POSTIVE,COORD_POSTIVE };
}
View::View(View* parent) : View()
{
	size = { 32,32 };
	this->parent = parent;
	root = parent->root;
	UpdateTransform();
	gD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &render.context);
	render.context->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	auto hr = render.context->CreateLayer(&render.clipLayer);
	if (hr != S_OK)
	{
		throw new exception();
	}
	//render.context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
	SendEvent(FE_SIZED, 0, 0);
	parent->children.push_back(this);
	parent->SendEvent(FE_CHILD, (WPARAM)this, 4);

}

static ID2D1DeviceContext1* currentRootCtx;

LRESULT View::SendEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	ID2D1DeviceContext1* tempRootCtx = nullptr;
	LRESULT ret = 0;
	switch (msg)
	{
	case FE_PAINT:
	{
		if (render.alpha == 1) break;
		//render.direct = true;
		D2D1_MATRIX_3X2_F transform = *(D2D1_MATRIX_3X2_F*)wParam;
		if (render.grouped)
		{
			tempRootCtx = currentRootCtx;
			currentRootCtx = render.context;
		}
		else
		{
			currentRootCtx = (ID2D1DeviceContext1*)lParam;
			transform = localTransform * transform;
		}
		currentRootCtx->SetTransform(transform);
		if (masked)
		{
			if (render.alpha == 0)
			{
				currentRootCtx->PushAxisAlignedClip({ 0,0,rect.width(),rect.height() }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			}
			else
			{
				//currentRootCtx->CreateLayer({ rect.width(),rect.height() }, &render.clipLayer);

				currentRootCtx->PushLayer(
					D2D1::LayerParameters1({ 0,0,rect.width(),rect.height() }
						, 0
						, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
						, D2D1::IdentityMatrix()
						, 1 - render.alpha
					), render.clipLayer);
			}
		}
		//currentRootCtx->PushAxisAlignedClip({ 0,0,rect.width(),rect.height() }, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		if (render.dirty || render.direct)
			Draw();
		if (!render.direct && !render.container)
		{
			D2D1_POINT_2F ofs{ 0,0 };
			ofs = ofs * transform;
			float x, y;
			x = ofs.x * dpiScaleFactor.x;
			y = ofs.y * dpiScaleFactor.y;
			x = ((int)x - x) / dpiScaleFactor.x;
			y = ((int)y - y) / dpiScaleFactor.y;

			IDXGISurface* ds;
			auto hr = render.content->QueryInterface(&ds);

			auto sz = render.content->GetSize();
			//std::cout << "indirect bitblt:" << sz.width << "," << sz.height << std::endl;
			currentRootCtx->DrawBitmap(render.content, D2D1::RectF(x, y, x + sz.width / dpiScaleFactor.x, y + sz.height / dpiScaleFactor.y));
		}
		if (Debug::showBorder)
		{
			ID2D1SolidColorBrush* br;
			currentRootCtx->CreateSolidColorBrush(ColorF::ColorF(ColorF::Fuchsia), &br);
			D2D1_SIZE_F sz;
			if (render.container)
				sz = { rect.width() * dpiScaleFactor.x,rect.height() * dpiScaleFactor.y };
			else
				sz = render.content->GetSize();
			currentRootCtx->DrawRectangle({ 0,0,sz.width / dpiScaleFactor.x,sz.height / dpiScaleFactor.y }, br);
			if (root->mouseFoci == this)
			{
				br->SetColor(ColorF::ColorF(ColorF::Orange));
				currentRootCtx->DrawRectangle({ 2,2,sz.width / dpiScaleFactor.x - 2,sz.height / dpiScaleFactor.y - 2 }, br);
			}
			if (root->keyboadrFoci == this)
			{
				br->SetColor(ColorF::ColorF(ColorF::DarkSeaGreen));
				currentRootCtx->DrawRectangle({ 4,4,sz.width / dpiScaleFactor.x - 4,sz.height / dpiScaleFactor.y - 4 }, br);
			}
			br->Release();
		}

		for (auto i : children)
		{
			i->SendEvent(msg, (WPARAM)&transform, (LPARAM)currentRootCtx);
		}
		if (masked)
		{
			if (render.alpha == 0)
				currentRootCtx->PopAxisAlignedClip();
			else
			{
				currentRootCtx->PopLayer();
			}
		}
		//currentRootCtx->PopAxisAlignedClip();
		if (render.grouped)
		{
			currentRootCtx = tempRootCtx;
		}
		break;
	}
	case FE_SIZED:
	{
		UpdateView();
		if (layout.layout)
			layout.layout->Layout(children);
		if (render.context && !render.container)
		{
			EnterCriticalSection(&gThreadAccess);
			if (render.sharedSurface)
				render.sharedSurface->Release();
			if (render.content)
				render.content->Release();
			//std::cout<< "content release ref c:" << render.content->Release() << std::endl;
			D2D1_BITMAP_PROPERTIES1 bp = BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET,
				PixelFormat(
					DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
			D2D1_SIZE_U sz = D2D1::SizeU(ceilf((rect.right - rect.left) * dpiScaleFactor.x), ceilf((rect.bottom - rect.top) * dpiScaleFactor.y));
			if (sz.width <= 0 || sz.width > 16384) sz.width = 1;
			if (sz.height <= 0 || sz.height > 16384) sz.height = 1;
			DXGI_SURFACE_DESC dscDS;
			dscDS.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			dscDS.SampleDesc.Count = 1;
			dscDS.SampleDesc.Quality = 0;
			dscDS.Width = sz.width;
			dscDS.Height = sz.height;

			/*auto hr = gDXGIDevice->CreateSurface(&dscDS, 1, DXGI_USAGE_SHARED | DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT, nullptr, &render.sharedSurface);

			ID3D11Resource* resc;
			hr = render.sharedSurface->QueryInterface(&resc);
			if (hr != S_OK)
				throw new exception();
			hr = render.context->CreateSharedBitmap(__uuidof(IDXGISurface), render.sharedSurface,
				&BitmapProperties(PixelFormat(
					DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), (ID2D1Bitmap**)&render.content);*/
			render.context->CreateBitmap(sz, nullptr, 0, bp, &render.content);
			render.context->SetTarget(render.content);
			render.context->SetDpi(96 * dpiScaleFactor.x, 96 * dpiScaleFactor.y);
			LeaveCriticalSection(&gThreadAccess);
			OnEvent(msg, wParam, lParam);
		}
		if (parent) parent->SendEvent(FE_CHILDSIZED, 0, lParam);
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			(*i)->SendEvent(FE_PARENTSIZED, wParam, lParam);
		}
		break;
	}
	case FE_MOVED:
	{
		if (parent) parent->SendEvent(FE_CHILDMOVED, 0, lParam);
		break;
	}
	case FE_MOUSEMOVE:
	{
		D2D1_POINT_2F& mouse = *(D2D1_POINT_2F*)wParam;
		if (lParam == 0xffff0fff)
		{
			auto sr = OnEvent(FE_MOUSEMOVE, 0, ((int)mouse.y << 16) | (int)mouse.x);
			if (sr) return sr;
			return (LRESULT)this;

		}
		bool catched = false;
		for (auto i = children.rbegin(); i != children.rend(); ++i)
		{
			if (!(*i)->disabled)
			{
				if ((*i)->custommousetest)
				{
					D2D1_POINT_2F fixed{ mouse.x - (*i)->rect.left, mouse.y - (*i)->rect.top };
					ret = (*i)->SendEvent(FE_MOUSEHIT, (WPARAM)&fixed, 0);
					if (ret != 0)
					{
						ret = (*i)->SendEvent(msg, (WPARAM)&fixed, 0);
						if (ret != 0)
						{
							catched = true;
							break;
						}
						break;
					}

				}
				else
				{
					if (PointInRect(mouse, (*i)->rect))
					{
						D2D1_POINT_2F fixed{ mouse.x - (*i)->rect.left, mouse.y - (*i)->rect.top };
						ret = (*i)->SendEvent(msg, (WPARAM)&fixed, 0);
						if (ret != 0)
						{
							catched = true;
							break;
						}
					}
				}
			}
		}
		if (!catched && mouseable)
		{
			root->SetMouseFoci(this);

			unsigned short my = (short)mouse.y;
			unsigned short mx = (short)mouse.x;
			auto sr = OnEvent(FE_MOUSEMOVE, 0, ((int)my << 16) | (int)mx);
			if (sr) return sr;
			return (LRESULT)this;
		}
		else if (monitor)
			OnEvent(msg, wParam, lParam);
		return ret;

	}
	case FE_ANIMATION:
	{
		AnimationData& a = *(AnimationData*)wParam;
		Animation(*(float*)lParam, a.wParam, a.lParam);
		break;
	}
	case FE_SCROLL:
	{
		if (mouseable)
		{
			ret = OnEvent(msg, wParam, lParam);
			if (!ret && parent && !(lParam & FE_S_L_FROMPARENT))
			{
				parent->SendEvent(msg, wParam, lParam);
			}
		}
		break;
	}
	case FE_RADIO_BROADCAST:
	{
		for (vector<View*>::iterator i = children.begin(); i != children.end(); ++i)
		{
			(*i)->SendEvent(FE_RADIO, wParam, lParam);
		}
		break;
	}
	case FE_CHILD:
	case FE_CHILDSIZED:
	case FE_CHILDMOVED:
	{
		if (msg == FE_CHILDSIZED)
			if (parent) parent->SendEvent(FE_CHILDSIZED, 0, lParam);

		if (layout.layout)
			layout.layout->Layout(children);
		if (layout.sizeMode.x == SIZE_MODE_CHILDREN || layout.sizeMode.y == SIZE_MODE_CHILDREN)
		{
			ret = OnEvent(msg, wParam, lParam);
			UpdateTransform();
		}
		break;
	}
	case FE_PARENTSIZED:
	{
		if (layout.coord.x == COORD_FILL || layout.coord.y == COORD_FILL)
		{
			ret = OnEvent(msg, wParam, lParam);
			UpdateTransform();
		}
		break;
	}
	case FE_DESTROY:
	{

		disposed = true;
		for (auto i = children.begin(); i != children.end();)
		{
			(*i)->SendEvent(FE_DESTROY, 0, 0);
			i = children.erase(i);
		}
		OnEvent(msg, wParam, lParam);
		//if (parent)parent->RemoveChild(this);
		if (render.content)
		{
			render.content->Release();
			render.content = nullptr;
		}
		if (render.context)
		{
			render.context->Release();
			render.context = nullptr;
		}
		break;
	}
	default:
		ret = OnEvent(msg, wParam, lParam);
	}
	for (auto i = listeners.begin(); i != listeners.end(); ++i)
	{
		if ((*i)->event == msg)
		{
			if ((*i)->object)
				(*i)->method((*i)->object, this, msg, wParam, lParam);
			else
				(*i)->function(this, msg, wParam, lParam);
		}
	}
	return ret;
}

ID2D1DeviceContext1* View::BeginDraw(const D2D1_COLOR_F& clear, bool dc)
{
	//currentRootCtx->SetTransform(finalTransform);
	if (render.direct)
	{
		if (!dc)
		{
			ID2D1SolidColorBrush* br;
			currentRootCtx->CreateSolidColorBrush(clear, &br);
			auto pt = Position();
			currentRootCtx->FillRectangle(D2D1::RectF(0, 0, rect.width(), rect.height()), br);
			//ctx->FillRectangle(D2D1::RectF(50,50,100,100), br);
			br->Release();
		}
	}
	else
	{
		render.context->BeginDraw();
		if (!dc)
			render.context->Clear(clear);
		return render.context;
	}
	return currentRootCtx;
}


/*LRESULT View::TestCallback(View* view, Message msg, WPARAM wParam, LPARAM lParam)
{
	std::cout << "i am a method callback, object::" << this << "params: "
		<< msg << "," << wParam << "," << lParam
		<< std::endl;
	return 1234;
}*/
RootView* View::Root()
{
	return root;
}

View* View::Parent()
{
	return parent;
}
void View::Parent(View* newParent)
{
	if (newParent == this) return;
	parent->RemoveChild(this);
	newParent->AppendChild(this);
	UpdateTransform();
}
void View::EndDraw()
{
	render.dirty = false;
	if (render.direct)
	{
		//EventListener l;
		//l.method = MakeListenerCallback(&View::TestCallback);
	}
	else
	{

		auto hr = render.context->EndDraw();
		if (hr != S_OK)
			OutputDebugString(L"Error EndDraw\n");
#ifdef _DX12
		//render.context->Flush();

#endif
		//std::cout << "Error EndDraw:" << hr << std::endl;
	}

}

static UINT32 reversive = 0;
void View::UpdateTransform()
{
	//if (!layout.dirty) return;
	reversive++;
	if (reversive > 4) { reversive--; return; }
	auto prerect = rect;
	rect = place();
	if (prerect.width() != rect.width() || prerect.height() != rect.height())
		SendEvent(FE_SIZED, 0, 4);
	if (prerect.left != rect.left || prerect.top != rect.top)
		SendEvent(FE_MOVED, 0, 4);
	auto anchor = layout.anchor;
	anchor.x = anchor.x * rect.width();
	anchor.y = anchor.y * rect.height();
	localTransform = Matrix3x2F::Scale(scale, anchor) * Matrix3x2F::Translation(rect.left, rect.top);
	if (parent)
		finalTransform = localTransform * parent->finalTransform;
	else
		finalTransform = localTransform;
	reversive--;
	for (vector<View*>::iterator i = children.begin(); i != children.end(); ++i)
	{
		(*i)->UpdateTransform();
	}

	layout.dirty = false;
}

void View::UpdateView()
{
	render.dirty = true;
	if (parent == nullptr || this == root)
		return;

	parent->UpdateView();
}

void View::Size(D2D1_SIZE_F size)
{
	this->size = size;
	UpdateTransform();
	//UpdateView();
}

D2D1_POINT_2F View::Position(bool absolute)
{
	D2D1_POINT_2F pt{ 0,0 };

	if (absolute)
		return pt * finalTransform;
	return position;
}
D2D1_SIZE_F View::Size()
{
	return size;
}

void View::Position(D2D1_POINT_2F relativePosition, bool update)
{
	position = relativePosition;
	if (update)
		UpdateTransform();
}

animation_id View::Animate(time_t duration, int param1, int param2, int id_override)
{
	return FlameUI::Animate(this, duration, param1, param2, id_override);
}

D2D1_RECT_F View::place()
{
	D2D1_RECT_F res{ 0,0,0,0 };
	float w = 0, h = 0, half = 0, mid, t;
	bool empty = true;
	if (!parent) return { 0,0,size.width,size.height };
	if (layout.sizeMode.x == SIZE_MODE_CHILDREN || layout.sizeMode.y == SIZE_MODE_CHILDREN)
	{
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			View* child = (*i);
			if (child->render.alpha < 1)
			{
				if (child->layout.coord.x != COORD_FILL)
				{
					t = child->rect.left + child->rect.width() +
						child->layout.box.margin.right + layout.box.padding.right;
					if (t > w) w = t;
					empty = false;
				}
				if (child->layout.coord.y != COORD_FILL)
				{
					t = child->rect.top + child->rect.height() +
						child->layout.box.margin.bottom + layout.box.padding.bottom;
					if (t > h) h = t;
					empty = false;
				}
			}
		}
		if (empty)
		{
			if (layout.coord.x == COORD_POSTIVE)
				w = parent->rect.width() - parent->layout.box.padding.left - layout.box.margin.left - position.x;
			else
				w = parent->rect.width();
			if (layout.coord.y == COORD_POSTIVE)
				h = parent->rect.height() - parent->layout.box.padding.top - layout.box.margin.top - position.y;
			else
				h = parent->rect.height();
		}
		if (layout.sizeMode.x == SIZE_MODE_CHILDREN)
			size.width = w;
		if (layout.sizeMode.y == SIZE_MODE_CHILDREN)
			size.height = h;

	}
	switch (layout.coord.x)
	{
	case COORD_POSTIVE:
		res.left = position.x + layout.box.margin.left + parent->layout.box.padding.left;
		res.right = res.left + size.width;
		break;
	case COORD_NEGATIVE:
		res.right = parent->rect.width() - position.x - layout.box.margin.right - parent->layout.box.padding.right;
		res.left = res.right - size.width;
		break;
	case COORD_CENTER:
		mid = (parent->rect.width() - parent->layout.box.padding.left - parent->layout.box.padding.right) / 2 +
			parent->layout.box.padding.left;
		half = size.width / 2;
		res.left = mid - half + position.x;
		res.right = mid + half + position.x;
		break;
	case COORD_FILL:
		if (layout.sizeMode.x == SIZE_MODE_NONE)
		{
			res.left = position.x + layout.box.margin.left + parent->layout.box.padding.left;
			res.right = parent->rect.width() - size.width - layout.box.margin.right - parent->layout.box.padding.right;
		}
		break;
	}

	switch (layout.coord.y)
	{
	case COORD_POSTIVE:
		res.top = position.y + layout.box.margin.top + parent->layout.box.padding.top;
		res.bottom = res.top + size.height;
		break;
	case COORD_NEGATIVE:
		res.bottom = parent->rect.height() - position.y - layout.box.margin.bottom - parent->layout.box.padding.bottom;
		res.top = res.bottom - size.height;
		break;
	case COORD_CENTER:
		mid = (parent->rect.height() - parent->layout.box.padding.top - parent->layout.box.padding.bottom) / 2 +
			parent->layout.box.padding.top;
		half = size.height / 2;
		res.top = mid - half + position.y;
		res.bottom = mid + half + position.y;
		break;
	case COORD_FILL:
		if (layout.sizeMode.y == SIZE_MODE_NONE)
		{
			res.top = position.y + layout.box.margin.top + parent->layout.box.padding.top;
			res.bottom = parent->rect.height() - size.height - layout.box.margin.bottom - parent->layout.box.padding.bottom;
		}
		break;
	}

	return res;
}

D2D1_SIZE_F View::area()
{
	return { 0
			,0 };
}

void View::Coord(COORD x, COORD y)
{
	if (x != COORD_RESERVE)
		layout.coord.x = x;
	if (y != COORD_RESERVE)
		layout.coord.y = y;
	UpdateTransform();
}
struct Coord View::Coord()
{
	return layout.coord;
}

void View::SizeMode(SIZE_MODE x, SIZE_MODE y)
{
	layout.sizeMode = { x,y };
}

void View::UpdateAll()
{
	UpdateView();
	for (vector<View*>::iterator i = children.begin(); i != children.end(); ++i)
	{
		(*i)->UpdateAll();
	}

}

View* View::FindChildFoci(View* after, bool& contains)
{
	View* ret = nullptr;
	bool sel = after == nullptr;

	for (auto i = children.begin(); i != children.end(); ++i)
	{
		View& child = *(*i);
		if (child.keyable && (sel || contains))
		{
			ret = &child;
			break;
		}
		if (after == &child) sel = true;
	}
	View* x = nullptr, * mc = nullptr;
	if (ret == nullptr)
	{
		for (auto i = children.begin(); i != children.end(); ++i)
		{
			View& child = *(*i);
			if (contains)
			{
				after = nullptr;
				if (child.keyable)
					x = &child;
			}
			if (!x)
			{
				mc = child.FindChildFoci(after, contains);
			}
			if (x)
			{
				ret = x;
				break;
			}
			if (mc)
			{
				ret = mc;
				break;
			}
		}
	}
	if (!ret && after && !contains)
		contains = sel;
	return ret;
}

float View::Alpha()
{
	return render.alpha;
}
void View::Alpha(float a)
{
	if (a < 0) a = 0;
	if (a > 1) a = 1;
	render.alpha = a;
}
FlameUI::Rect View::Margin()
{
	return layout.box.margin;
}
void View::Margin(Rect m, bool update)
{
	layout.box.margin = m;
	if (update) UpdateTransform();
}
FlameUI::Rect View::Padding()
{
	return layout.box.padding;
}
void View::Padding(Rect m, bool update)
{
	layout.box.padding = m;
	if (update) UpdateTransform();
}

void View::Anchor(D2D1_POINT_2F a)
{
	layout.anchor = a;
	UpdateTransform();
}
D2D1_POINT_2F View::Anchor()
{
	return layout.anchor;
}

void View::Scale(D2D1_SIZE_F a)
{
	scale = a;
	UpdateTransform();
	UpdateView();
}
D2D1_SIZE_F View::Scale()
{
	return scale;
}
FlameUI::Rect View::GetRect()
{
	return rect;
}
void View::Layouter(Placer* p)
{
	if (layout.layout)
		delete layout.layout;
	layout.layout = p;
}
D2D1_MATRIX_3X2_F View::FinalTransform()
{
	return finalTransform;
}
bool View::RemoveChild(View* c)
{
	bool ret = false;
	for (auto i = children.begin(); i != children.end(); i++)
	{
		if ((*i) == c)
		{
			children.erase(i);
			ret = true;
			break;
		}
	}
	return ret;
}
void View::ResetRoot(RootView* r)
{
	root = r;
	for (auto i : children)
	{
		i->ResetRoot(r);
	}

}
void View::AppendChild(View* c)
{
	children.push_back(c);
	c->parent = this;
	c->ResetRoot(this->root);
}

bool View::Contains(View* v, bool recursive)
{
	bool ret = false;
	for (auto i = children.begin(); i != children.end(); i++)
	{
		if ((*i) == v)
		{
			ret = true;
			break;
		}
	}
	if (recursive)
	{
		for (auto i = children.begin(); i != children.end(); i++)
		{
			if ((*i)->Contains(v, true))
			{
				ret = true;
				break;
			}
		}
	}
	return ret;
}

int View::ZOrder()
{
	return z;
}
void View::ZOrder(int zo, bool update)
{
	z = zo;
	if (parent && update)
		parent->UpdateZ();
}
void View::UpdateZ()
{
	sort(children.begin(), children.end(), [](const View* a, const View* b) {return a->z < b->z; });
	UpdateView();
}
bool View::operator<(const View& v)
{
	return z < v.z;
}
bool View::operator>(const View& v)
{
	return z > v.z;
}
void View::Destroy()
{

	if (parent)
	{
		parent->RemoveChild(this);
		SendEvent(FE_DESTROY, 0, 0);
	}
	else if (dynamic_cast<Frame*>(this))
	{
		((Frame*)this)->Close();
	}
}
bool View::Disposed()
{
	return disposed;
}

vector<View*> View::GetChildren()
{
	return children;
}
