#include "FlirecViewer.h"
#include "FlameUI.h"
#include "winuser.h"
#include "windows.h"
#include "FlameUIBasics.h"
#include "Chart.h"
#include <fstream>
using namespace FlameUI;
using namespace D2D1;
Chart* ca, * cg, * cm, * cq;
Frame* fvw = nullptr;
Scroller* sb;
void ShowFilrecViewerWindow()
{
	if (fvw)
	{
		FlashWindow(fvw->GetNative(), true);
		return;
	}
	fvw = new Frame({ 900,600 });
	fvw->Title(L"飞行记录");
	//FlameUI::Debug::showBorder = true;
	Button* btn;
	Panel* pnl = new Panel(fvw);
	pnl->Coord(COORD_NEGATIVE, COORD_NEGATIVE);
	pnl->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	pnl->Position({ 5,5 });
	pnl->Layouter(new LinearPlacer(DIRECTION_HORIZONTAL, 5));

	btn = new Button(pnl);
	btn->Padding({ 10,5,10,5 });
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"导入记录");
	btn->AddEventListener([](Message, WPARAM, LPARAM)
		{
			TCHAR szBuffer[MAX_PATH] = { 0 };
			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);//同样给出需要定义打开需要的结构体
			ofn.lpstrFilter = L"FLYC文件(*.flyc)\0*.flyc\0";//要选择的文件后缀 
			ofn.lpstrFile = szBuffer;//存放文件的缓冲区 
			ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
			ofn.nFilterIndex = 0;
			ofn.lpstrTitle = TEXT("选择文件");
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;//标志如果是多选要加上OFN_ALLOWMULTISELECT
			if (!GetOpenFileName(&ofn))
			{
				return;
			}

			std::ifstream fs(szBuffer, std::ios::in | std::ios::binary);
			fs.seekg(0x44);
			int cnt = 0;
			while (!fs.eof())
			{
				float f;
				fs.read((char*)&f, sizeof f);
				cg->JoinValue(0, f, false);
				fs.read((char*)&f, sizeof f);
				cg->JoinValue(1, f, false);
				fs.read((char*)&f, sizeof f);
				cg->JoinValue(2, f, false);

				fs.read((char*)&f, sizeof f);
				ca->JoinValue(0, f, false);
				fs.read((char*)&f, sizeof f);
				ca->JoinValue(1, f, false);
				fs.read((char*)&f, sizeof f);
				ca->JoinValue(2, f, false);

				fs.read((char*)&f, sizeof f);
				cq->JoinValue(0, f, false);
				fs.read((char*)&f, sizeof f);
				cq->JoinValue(1, f, false);
				fs.read((char*)&f, sizeof f);
				cq->JoinValue(2, f, false);
				fs.read((char*)&f, sizeof f);
				cq->JoinValue(3, f, false);
				cnt++;

			}
			sb->Max(cnt+2000);
			cg->GenerateCurve(0);
			cg->GenerateCurve(1);
			cg->GenerateCurve(2);
			ca->GenerateCurve(0);
			ca->GenerateCurve(1);
			ca->GenerateCurve(2);
			cq->GenerateCurve(0);
			cq->GenerateCurve(1);
			cq->GenerateCurve(2);
			cq->GenerateCurve(3);
			cm->GenerateCurve(0);
			cm->GenerateCurve(1);
			cm->GenerateCurve(2);

		}, FE_LBUTTONDOWN);

	btn = new Button(pnl);
	btn->Padding({ 10,5,10,5 });
	btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
	new Label(btn, L"没用的按钮");

	ScrollView* sv;
	sv = new ScrollView(fvw);
	sv->Coord(COORD_FILL, COORD_FILL);
	sv->Position({ 5, 32 });
	sv->Size({ 5,55 });
	sv->Content()->Layouter(new LinearPlacer(DIRECTION_VERTICAL, 8));

	pnl = sv->Content();
	pnl->Coord(COORD_FILL, COORD_POSITIVE);
	pnl->SizeMode(SIZE_MODE_NONE, SIZE_MODE_CHILDREN);
	pnl->Size({ 15,100 });

	new Label(pnl, L"加速度");
	ca = new Chart(pnl);
	ca->Coord(COORD_FILL, COORD_POSITIVE);
	ca->Size({ 0, 250 });
	ca->JoinSeries(L"x", ColorF(ColorF::Red));
	ca->JoinSeries(L"y", ColorF(ColorF::Green));
	ca->JoinSeries(L"z", ColorF(ColorF::Blue));
	ca->beginIndex = 0;

	new Label(pnl, L"角速度");
	cg = new Chart(pnl);
	cg->Coord(COORD_FILL, COORD_POSITIVE);
	cg->Size({ 0, 250 });
	cg->JoinSeries(L"x", ColorF(ColorF::Red));
	cg->JoinSeries(L"y", ColorF(ColorF::Green));
	cg->JoinSeries(L"z", ColorF(ColorF::Blue));
	cg->beginIndex = 0;

	new Label(pnl, L"磁感应强度");
	cm = new Chart(pnl);
	cm->Coord(COORD_FILL, COORD_POSITIVE);
	cm->Size({ 0, 250 });
	cm->JoinSeries(L"x", ColorF(ColorF::Red));
	cm->JoinSeries(L"y", ColorF(ColorF::Green));
	cm->JoinSeries(L"z", ColorF(ColorF::Blue));
	cm->beginIndex = 0;


	new Label(pnl, L"姿态四元数");
	cq = new Chart(pnl);
	cq->Coord(COORD_FILL, COORD_POSITIVE);
	cq->Size({ 0, 250 });
	cq->JoinSeries(L"w", ColorF(ColorF::Blue));
	cq->JoinSeries(L"x", ColorF(ColorF::Red));
	cq->JoinSeries(L"y", ColorF(ColorF::Green));
	cq->JoinSeries(L"z", ColorF(ColorF::Yellow));
	cq->beginIndex = 0;


	sb = new Scroller(fvw);
	sb->Coord(COORD_FILL, COORD_NEGATIVE);
	sb->Position({ 5,40 });
	sb->Size({ 5,15 });
	sb->horizontal = true;
	sb->AddEventListener(
		[](Message, WPARAM, LPARAM)
		{
			ca->beginIndex = sb->Offset();
			cg->beginIndex = sb->Offset();
			cm->beginIndex = sb->Offset();
			cq->beginIndex = sb->Offset();

			cg->GenerateCurve(0);
			cg->GenerateCurve(1);
			cg->GenerateCurve(2);
			ca->GenerateCurve(0);
			ca->GenerateCurve(1);
			ca->GenerateCurve(2);
			cq->GenerateCurve(0);
			cq->GenerateCurve(1);
			cq->GenerateCurve(2);
			cq->GenerateCurve(3);
			//cm->GenerateCurve(0);
			//cm->GenerateCurve(1);
			//cm->GenerateCurve(2);

		}, FE_S_SCROLLING);

	fvw->AddEventListener([](Message, WPARAM, LPARAM) {
		fvw = nullptr;
		}, FE_DESTROY);

	fvw->Show();

}

void OpenFR()
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);//同样给出需要定义打开需要的结构体
	ofn.lpstrFilter = L"FLYC文件(*.flyc)\0*.flyc\0";//要选择的文件后缀 
	ofn.lpstrFile = szBuffer;//存放文件的缓冲区 
	ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
	ofn.nFilterIndex = 0;
	ofn.lpstrTitle = TEXT("选择文件");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;//标志如果是多选要加上OFN_ALLOWMULTISELECT
	if (!GetOpenFileName(&ofn))
	{
		return;
	}
}