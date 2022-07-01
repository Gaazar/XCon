#include "FlameUI.h"
#include "Frame.h"
#include "Label.h"
#include "Button.h"
#include "Image.h"
#include "Scroller.h"
#include "ScrollView.h"
#include "CheckBox.h"
#include "Toggle.h"
#include "RadioButton.h"
#include "TextEditor.h"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
using namespace FlameUI;
using namespace std;

#include "LinearPlacer.h"
#include "SectionBar.h"
#include "SheetView.h"
#include "MenuFrame.h"
#include "DockProvider.h"
#include "SeperatorHandle.h"
#include "ImGuiCanvas.h"
#include "VideoPlayer.h"
#include "XInputCehcker.h"

#include "json.hpp"




int main()
{
	configor::wjson j;
	j[L"version"] = L"v0.1b";
	WinMain(0, 0, nullptr, 0);

	return 0;
}



int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	Initiate();
	//Test();
	//return 0;
	Frame mainFrame({ 1280,720 });
	//mainFrame.AddEventListener(0, &cb_close, FE_DESTROY);
	mainFrame.Title(L"XCon");
	VideoPlayer vp(&mainFrame);
	vp.Size({ 1260,576 });
	vp.Position({ 10,40 });
	vp.Source(L"udp://@192.168.1.5:11451");
	//vp.Source(L"N:\\Video\\2022-06-29 14-56-30.mp4");
	//vp.Source(L"D:\\Videos\\vnv.mp4");

	XInputCehcker xic(&mainFrame);
	xic.Position({ 50,50 });
	mainFrame.Show();
	mainFrame.MainLoop();
	//libm.Commit();
	return 0;
}