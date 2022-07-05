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
#include <thread>

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
#include "XInput.h"

#include "Transmission.h"
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

bool cexit = false;
Chart* tchart = nullptr;

Chart* crtaccl;
Chart* crtgyro;
Chart* crtmagt;

void Controls()
{
	while (!cexit)
	{
		XINPUT_STATE s;
		if (XInputGetState(0, &s) == ERROR_SUCCESS)
		{
			ControlPack cp;
			cp.accelerator = -s.Gamepad.sThumbLY - 1;
			cp.yaw = -s.Gamepad.bLeftTrigger + s.Gamepad.bRightTrigger;
			cp.pitch = s.Gamepad.sThumbRX;
			cp.roll = -s.Gamepad.sThumbRY - 1;
			RunInUIThread([cp]()
				{
					tchart->JoinValue(0, cp.yaw / 255.f);
					tchart->JoinValue(1, cp.pitch / 32768.f);
					tchart->JoinValue(2, cp.roll / 32768.f);
					tchart->JoinValue(3, cp.accelerator / 32768.f);
				});
			SendPacket(cp, "192.168.1.7", 10485);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}
void OnRecvTransmisson(int len, char* buff)
{
	Packet p;
	memcpy(&p, buff, len);
	RunInUIThread([p]()
		{
			crtaccl->JoinValue(0, p.feedback.gyro.x);
			crtaccl->JoinValue(1, p.feedback.gyro.y);
			crtaccl->JoinValue(2, p.feedback.gyro.z);

			crtgyro->JoinValue(0, p.feedback.gyro.yaw);
			crtgyro->JoinValue(1, p.feedback.gyro.pitch);
			crtgyro->JoinValue(2, p.feedback.gyro.roll);

			crtmagt->JoinValue(0, p.feedback.magnetometer.x);
			crtmagt->JoinValue(1, p.feedback.magnetometer.y);
			crtmagt->JoinValue(2, p.feedback.magnetometer.z);

		});

}
int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	InitTransmission(10058);
	Initiate();
	//Test();
	//return 0;
	Frame mainFrame({ 1280,720 });
	//mainFrame.AddEventListener(0, &cb_close, FE_DESTROY);
	mainFrame.Title(L"XCon");
	VideoPlayer vp(&mainFrame);
	vp.Size({ 1260,576 });
	vp.Position({ 10,40 });
	vp.Source(L"udp://@239.0.0.1:11451");
	//vp.Source(L"N:\\Video\\2022-06-29 14-56-30.mp4");
	//vp.Source(L"D:\\Videos\\vnv.mp4");

	Chart crt(&mainFrame);
	tchart = &crt;
	crt.Size({ 800,350 });
	crt.Position({ 20,100 });
	crt.JoinSeries(L"Yaw", ColorF(ColorF::Red));
	crt.JoinSeries(L"Pitch", ColorF(ColorF::Green));
	crt.JoinSeries(L"Roll", ColorF(ColorF::Blue));
	crt.JoinSeries(L"Accelerator", ColorF(ColorF::Yellow));

	mainFrame.AddEventListener([](Message, WPARAM, LPARAM) {

		cexit = true;
		exit(0);
		}, FE_DESTROY);
	mainFrame.Show();
	Frame fbFrame({ 1000, 820 });
	Chart crtacc(&fbFrame);
	crtacc.Size({ 960,250 });
	crtacc.Position({ 20,40 });
	crtacc.JoinSeries(L"Accl X", ColorF(ColorF::Red));
	crtacc.JoinSeries(L"Accl Y", ColorF(ColorF::Green));
	crtacc.JoinSeries(L"Accl Z", ColorF(ColorF::Blue));
	crtaccl = &crtacc;

	Chart crtgry(&fbFrame);
	crtgry.Size({ 960,250 });
	crtgry.Position({ 20,40 + 250 + 10 });
	crtgry.JoinSeries(L"Gyro X", ColorF(ColorF::Red));
	crtgry.JoinSeries(L"Gyro Y", ColorF(ColorF::Green));
	crtgry.JoinSeries(L"Gyro Z", ColorF(ColorF::Blue));
	crtgyro = &crtgry;

	Chart crtmag(&fbFrame);
	crtmag.Size({ 960,250 });
	crtmag.Position({ 20,40 + 260 + 260 });
	crtmag.JoinSeries(L"Magt X", ColorF(ColorF::Red));
	crtmag.JoinSeries(L"Magt Y", ColorF(ColorF::Green));
	crtmag.JoinSeries(L"Magt Z", ColorF(ColorF::Blue));
	crtmagt = &crtmag;

	fbFrame.Show();
	ShowInputCheckWindow();
	ShowControlWindow();

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Controls, 0, 0, nullptr);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlRecv, (LPVOID)OnRecvTransmisson, 0, nullptr);

	mainFrame.MainLoop();
	return 0;
}

