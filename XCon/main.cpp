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
#include "Chart.h"

#include "json.hpp"
#include "Frames.h"
#include "global.h"

configor::wjson configs = configor::wjson();


int main()
{
	wifstream ifs("./config.json");
	ifs >> configs;

	WinMain(0, 0, nullptr, 0);

	return 0;
}
void SaveConfigs()
{
	wofstream ofs("./config.json");
	ofs << configs;

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

	Chart crt(&mainFrame);
	crt.Size({ 800,350 });
	crt.Position({ 20,100 });

	mainFrame.AddEventListener([](Message, WPARAM, LPARAM) {
		exit(0);
		}, FE_DESTROY);
	mainFrame.Show();

	//ShowInputCheckWindow();
	//ShowControlWindow();
	mainFrame.MainLoop();
	//libm.Commit();
	return 0;
}