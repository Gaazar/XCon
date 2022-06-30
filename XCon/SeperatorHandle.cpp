#include "SeperatorHandle.h"
#include "FlameUI.h"
using namespace FlameUI;
using namespace D2D1;

void SeperatorHandle::Draw()
{
	if (pressed)
		BeginDraw(AdaptColor(FlameUI::Theme::Color::Background));
	else
		BeginDraw(D2D1::ColorF::ColorF(ColorF::Gray, 0.06f));
	EndDraw();
}
LRESULT SeperatorHandle::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case FE_LBUTTONDOWN:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		pressPos = { x,y };
		pressed = true;
		UpdateView();
	}
	break;
	case FE_LBUTTONUP:
		pressed = false;
		UpdateView();
		break;
	case FE_MOUSEMOVE:
	{
		if (pressed)
		{
			float x = GET_X_LPARAM(lParam);
			float y = GET_Y_LPARAM(lParam);
			auto cpos = Position();
			RunInUIThread([this, cpos, x, y]()
				{
					if (horizontal)
					{
						Position({ cpos.x, cpos.y + y - pressPos.y });
						if (top)
						{
							if (top->Coord().y == COORD_NEGATIVE)
							{
								top->Position({ top->Position().x,top->Parent()->GetRect().height() - rect.bottom - top->GetRect().height() });
							}
							else
								top->Position({ top->Position().x,rect.bottom });
						}
						if (bottom)
						{
							bottom->Size({ bottom->Size().width,rect.top - bottom->GetRect().top });
						}
					}
					else
					{
						Position({ cpos.x + x - pressPos.x , cpos.y });
						if (left)
						{
							left->Position({ rect.right, left->Position().y });
						}
						if (right)
						{
							right->Size({ rect.left - right->GetRect().left, right->Size().height });

						}
					}
					UpdateView();

				});
		}
	case WM_SETCURSOR:
	{
		if (horizontal)
			auto hr = SetCursor(LoadCursor(nullptr, IDC_SIZENS));
		else
			auto hr = SetCursor(LoadCursor(nullptr, IDC_SIZEWE));

		return 1;
	}
	break;
	}
	default:
		break;
	}
	return 0;
}

SeperatorHandle::SeperatorHandle(View* parent, bool horizontal) :View(parent)
{
	render.direct = true;
	this->horizontal = horizontal;
	if (horizontal)
	{
		Coord(COORD_FILL, COORD_POSTIVE);
		Size({ 0, FlameUI::Theme::LinePadding });
	}
	else
	{
		Coord(COORD_POSTIVE, COORD_FILL);
		Size({ FlameUI::Theme::LinePadding,0 });
	}
}

void SeperatorHandle::BindEdge(EDGE edge, View* view)
{
	switch (edge)
	{
	case FlameUI::EDGE_LEFT:
		left = view;
		//view->Coord(COORD_FILL, COORD_FILL);
		break;
	case FlameUI::EDGE_TOP:
		top = view;
		//view->Coord(COORD_FILL, COORD_FILL);
		break;
	case FlameUI::EDGE_RIGHT:
		right = view;
		view->Coord(COORD_POSTIVE, COORD_RESERVE);
		break;
	case FlameUI::EDGE_BOTTOM:
		bottom = view;
		view->Coord(COORD_RESERVE, COORD_POSTIVE);
		break;
	default:
		break;
	}
}

