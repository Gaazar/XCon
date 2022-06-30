#include "ImGuiCanvas.h"
#include "RootView.h"
#include "FlameUI.h"
#include <iostream>
using namespace FlameUI;
using namespace D2D1;
using namespace ImGui;

ImGuiCanvas::ImGuiCanvas(View* parent) :View(parent)
{
	IntereactPrepare();
	lockk = false;
}

LRESULT ImGuiCanvas::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	context.event = msg;
	switch (msg)
	{
	case FE_MOUSEMOVE:
	{
		float x = GET_X_LPARAM(lParam);
		float y = GET_Y_LPARAM(lParam);
		context.cursorPos = { x,y };
		//context.renderContext.ClearCommands();
		//context.renderContext.DrawEllipse({ x,y }, { 5,10 }, ColorF(ColorF::LemonChiffon));
		//context.renderContext.DrawEllipse({ x,y }, { 10,5 }, ColorF(ColorF::Goldenrod), 1.f);
		//context.renderContext.DrawLine({ 0,0 }, { x,y }, ColorF(ColorF::Teal));
		////context.renderContext.PushClipRect({ x / 2,y / 2,x / 2 + 100,y / 2 + 100 });
		//context.renderContext.DrawText(L"Deffered Render Context Text Drawing.", { x / 2,y / 2, x / 2 + 250, y / 2 + 250 }, ColorF(ColorF::White));
		////context.renderContext.PopClipRect();
		//context.renderContext.DrawRect({ 50,20 , 100,70 }, ColorF(ColorF::Khaki));
		//context.renderContext.DrawRect({ 80,60 , 180,100 }, ColorF(ColorF::RosyBrown), 1.f);

		//context.renderContext.DrawRoundedRect({ 150,120 , 200,170 }, { 15,10 }, ColorF(ColorF::Azure, 0.5f));
		//context.renderContext.DrawRoundedRect({ 180,160 , 230,200 }, { 10,10 }, ColorF(ColorF::Firebrick), 3.f);
		//UpdateView();
		IntereactPrepare();
		break;
	}
	case FE_LBUTTONDOWN:
	{
		context.lButtonDown = true;
		IntereactPrepare();
		break;
	}
	case FE_RBUTTONDOWN:
	{
		context.rButtonDown = true;
		IntereactPrepare();
		break;
	}
	case FE_MBUTTONUP:
	{
		context.mButtonDown = true;
		IntereactPrepare();
		break;
	}
	case FE_LBUTTONUP:
	{
		context.lButtonDown = false;
		IntereactPrepare();
		break;
	}
	case FE_RBUTTONUP:
	{
		context.rButtonDown = false;
		IntereactPrepare();
		break;
	}
	case FE_MBUTTONDOWN:
	{
		context.mButtonDown = false;
		IntereactPrepare();
		break;
	}
	default:
		break;
	}
	return 0;
}
void ImGuiCanvas::Draw()
{
	EnterCriticalSection(&gThreadAccess);
	auto ctx = BeginDraw({});
	context.renderContext.Excute(ctx, root->dTextFormat);
	EndDraw();
	lockk = false;
	LeaveCriticalSection(&gThreadAccess);
}
void ImGuiCanvas::IntereactPrepare()
{
	//while (lockk);
	//lockk = true;
	EnterCriticalSection(&gThreadAccess);
	defaultFont = root->dTextFormat;
	context.currentClipRect = { 0,0,rect.width(),rect.height() };
	context.drawPos = { context.currentClipRect.left,context.currentClipRect.top };
	context.renderContext.ClearCommands();
	context.prevX = 0;
	context.prevX = 0;
	context.seed = 0;
	currentCotext = &context;
	Intereact();
	LeaveCriticalSection(&gThreadAccess);
	lockk = false;
	UpdateView();
}
bool checkA = false;
bool checkB = false;

void ImGuiCanvas::Intereact()
{
	ImGui::Text(L"ImGui Canvas 😅", Theme::Color::Front, 2);
	if (ImGui::Button(L"Button"))
	{
		std::cout << "ImGu::Button Clicked" << std::endl;
	}
	ImGui::Button(L"Cancel", ColorF(ColorF::Red, 0.5f));
	ImGui::Seperator();
	ImGui::Text(L"SameLine() test.");
	ImGui::Button(L"Left"); ImGui::SameLine();
	ImGui::Button(L"Right");
	ImGui::Text(L" ");
	ImGui::Text(L"Text");
	ImGui::Seperator();
	ImGui::Text(L"Seperator");
	ImGui::Text(L"LinePadding");
	ImGui::CheckBox(L"ShowBorder", FlameUI::Debug::showBorder); ImGui::SameLine();
	ImGui::CheckBox(L"Ignore", checkB); ImGui::SameLine();
	if (ImGui::CheckBox(L"Light Theme", checkA))
	{
		if (checkA)
		{
			Theme::Color::Front = ColorF::ColorF(ColorF::Black);
			Theme::Color::Background = ColorF::ColorF(ColorF::AliceBlue);
			//Theme::Color::Theme = ColorF::ColorF(ColorF::DeepSkyBlue);
		}
		else
		{

			Theme::Color::Front = ColorF::ColorF(ColorF::White);
			Theme::Color::Background = ColorF::ColorF(0x00212121);
			//Theme::Color::Theme = ColorF::ColorF(ColorF::DeepSkyBlue);
		}
		ThemeUpdate();
	}

	if (FlameUI::Debug::showBorder)
	{
		ImGui::Text(L"Checked.");
	}
	if (!checkB)
	{
		ImGui::Text(L"Error!", ColorF(ColorF::Red));
		ImGui::Text(L"Warning", ColorF(ColorF::Orange));
		ImGui::Text(L"Info", ColorF(ColorF::Gray));
	}
	if (ImGui::CollapsingHeader(L"CollapsingHeader"))
	{
		ImGui::Text(L"Inside A!", ColorF(ColorF::Red)); ImGui::SameLine();
		ImGui::Text(L"Inside A!", ColorF(ColorF::Orange)); ImGui::SameLine();
		ImGui::Text(L"Inside A!", ColorF(ColorF::Gray));
	}
	if (ImGui::CollapsingHeader(L"#CollapsingHeader"))
	{
		ImGui::Text(L"Inside B!", ColorF(ColorF::Red));
		ImGui::Text(L"Inside B!", ColorF(ColorF::Orange));
		ImGui::Text(L"Inside B!", ColorF(ColorF::Gray));
	}
}
