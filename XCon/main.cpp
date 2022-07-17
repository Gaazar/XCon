#include "XCon.h"
#include "FlirecViewer.h"

using namespace FlameUI;
using namespace std;
void RxData(char* data, int len);

configor::wjson configs = configor::wjson();
#define REMOTE_IP "192.168.1.41"


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
Frame* ttframe;
Chart* tchart = nullptr;

Chart* crtaccl;
Chart* crtgyro;
Chart* crtmagt;
AttitudeMeeter* amtr;
AttitudeMeeter* amtr_mav;

float rollo = 0, pitcho = 0;
using namespace GxEngine;
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
			SendPacket(cp, REMOTE_IP, 10485);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}
void OnRecvTransmisson(int len, char* buff)
{
	Packet p;
	memcpy(&p, buff, len);
	if (p.type == PACKET_TYPE_MAVLINK)
	{
		RxData(p.mavlink.data, p.mavlink.meta.len);
		return;
	}
	if (p.type == PACKET_TYPE_FEEDBACK)
	{
		GxEngine::Vector3 ypr;
		Quaternion q;
		q.w = p.feedback.remote.attitude.w;
		q.x = p.feedback.remote.attitude.x;
		q.y = p.feedback.remote.attitude.y;
		q.z = p.feedback.remote.attitude.z;
		ypr.z = atan2f(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y)) * RAD2DEG;
		ypr.y = asinf(2 * (q.w * q.y - q.z * q.x)) * RAD2DEG;
		ypr.x = atan2f(2 * (q.x * q.y + q.w * q.z), 1 - 2 * (q.y * q.y + q.z * q.z)) * RAD2DEG;
		//std::cout << "Y/P/R: " << ypr.x << ", " << ypr.y << ", " << ypr.z << std::endl;
		RunInUIThread([p, ypr]()
			{
				/*crtaccl->JoinValue(0, p.feedback.gyro.x);
				crtaccl->JoinValue(1, p.feedback.gyro.y);
				crtaccl->JoinValue(2, p.feedback.gyro.z);

				crtgyro->JoinValue(0, p.feedback.gyro.yaw);
				crtgyro->JoinValue(1, p.feedback.gyro.pitch);
				crtgyro->JoinValue(2, p.feedback.gyro.roll);

				crtmagt->JoinValue(0, p.feedback.magnetometer.x);
				crtmagt->JoinValue(1, p.feedback.magnetometer.y);
				crtmagt->JoinValue(2, p.feedback.magnetometer.z);*/
				amtr->SetYPR(ypr.x, ypr.y + pitcho, ypr.z + rollo);

			});
	}

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
	vp.Coord(COORD_FILL, COORD_FILL);
	vp.Size({ 300,0 });
	vp.Position({ 0,32 });
	vp.Source(L"udp://@127.0.0.1:11451");
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

	/*Dropdown dp(&mainFrame);
	dp.Position({ 10,50 });
	dp.Size({ 150,FlameUI::Theme::LineHeight * 1.3f + FlameUI::Theme::LinePadding });*/

	amtr = new AttitudeMeeter(&mainFrame);
	amtr->Size({ 300,330 });
	amtr->Position({ 0,32 });
	amtr->Coord(COORD_NEGATIVE, COORD_POSITIVE);

	amtr_mav = new AttitudeMeeter(&mainFrame);
	amtr_mav->Size({ 300,330 });
	amtr_mav->Position({ 0,32 + 330 });
	amtr_mav->Coord(COORD_NEGATIVE, COORD_POSITIVE);

	{
		//Button* btn = new Button(&mainFrame);
		//btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		//btn->Padding({ 10,5,10,5 });
		//btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		//btn->Position({ 10, 370 });
		//new Label(btn, L"校准陀螺仪");
		//btn->AddEventListener([](Message, WPARAM, LPARAM)
		//	{
		//		CommandPack p;
		//		p.command = COMMAND_MPU_CALIBRATE;
		//		p.args[0] = 1;
		//		SendPacket(p, REMOTE_IP, 10485);
		//	}, FE_LBUTTONDOWN);

		//btn = new Button(&mainFrame);
		//btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		//btn->Padding({ 10,5,10,5 });
		//btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		//btn->Position({ 100, 370 });
		//new Label(btn, L"校准磁力计");
		//btn->AddEventListener([](Message, WPARAM, LPARAM)
		//	{
		//		CommandPack p;
		//		p.command = COMMAND_MPU_CALIBRATE;
		//		p.args[0] = 2;
		//		SendPacket(p, REMOTE_IP, 10485);
		//	}, FE_LBUTTONDOWN);

		//TextEditor* teor, * teop;
		//teop = new TextEditor(&mainFrame);
		//teop->Content(L"0");
		//teop->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		//teop->Position({ 120, 410 });
		//teop->Size({ 50, 26 });
		//btn = new Button(&mainFrame);
		//btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		//btn->Padding({ 10,5,10,5 });
		//btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		//btn->Position({ 10, 410 });
		//new Label(btn, L"设置俯仰偏移");
		//btn->AddEventListener([teop](Message, WPARAM, LPARAM)
		//	{
		//		pitcho = std::stof(teop->Content());
		//	}, FE_LBUTTONDOWN);

		//teor = new TextEditor(&mainFrame);
		//teor->Content(L"0");
		//teor->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		//teor->Position({ 120, 450 });
		//teor->Size({ 50, 26 });
		//btn = new Button(&mainFrame);
		//btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		//btn->Padding({ 10,5,10,5 });
		//btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		//btn->Position({ 10, 450 });
		//new Label(btn, L"设置横滚偏移");
		//btn->AddEventListener([teor](Message, WPARAM, LPARAM)
		//	{
		//		rollo = std::stof(teor->Content());

		//	}, FE_LBUTTONDOWN);

	}
	//fbFrame.Show();
	ttframe = &fbFrame;
	//ShowInputCheckWindow();
	//ShowControlWindow();
	//ShowFilrecViewerWindow();


	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Controls, 0, 0, nullptr);
	crt.mouseable = false;
	crt.Alpha(1);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlRecv, (LPVOID)OnRecvTransmisson, 0, nullptr);
	mainFrame.MainLoop();
	return 0;
}

void Amtr_Mav(float y, float p, float r)
{
	amtr_mav->SetYPR(y, p, r);
}
