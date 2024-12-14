#include "XCon.h"
#include "FlirecViewer.h"
#include "OSDOverlay.h"

using namespace FlameUI;
using namespace std;
void RxData(char* data, int len);

configor::wjson configs = configor::wjson();
#define REMOTE_IP "192.168.18.1"


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
OSDOverlay* osd;

float camplt_yaw;
float camplt_pitch;
bool camplt_mbd;
float camplt_sens = 0.002f;
FlameUI::Vector2 camplt_mbpos;

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
			//RunInUIThread([cp]()
			//	{
			//		tchart->JoinValue(0, cp.yaw / 255.f);
			//		tchart->JoinValue(1, cp.pitch / 32768.f);
			//		tchart->JoinValue(2, cp.roll / 32768.f);
			//		tchart->JoinValue(3, cp.accelerator / 32768.f);
			//	});
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
#include "DSPWindow.h"
void mavpxt();
int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

	InitTransmission(10058);
	//mavpxt();
	Initiate();
	//Test();
	//return 0;
	Frame mainFrame({ 1280,720 });
	//mainFrame.AddEventListener(0, &cb_close, FE_DESTROY);
	mainFrame.Title(L"XCon");
	//FlameUI::Debug::showBorder = true;

	MenuBar mb(&mainFrame);
	mb.Position({ 60, 5 });
	mb.SetMenu(Menu::Load(L"mainmenu.txt")[0]);
	mb.Callback([](Menu* m, int id)
		{
			if (id == 2)
			{
				ShowInputCheckWindow();
			}
			else if (id == 3)
			{
				ShowFilrecViewerWindow();
			}
			else if (id == 4)
			{
				CalibrateShow();
			}
			else if (id == 6)
			{
				ShowControlWindow();
			}

			//ShowControlWindow();
			//

			printf("menu id: %d\n", id);
		});

	VideoPlayer vp(&mainFrame);
	vp.Coord(COORD_FILL, COORD_FILL);
	vp.Size({ 300,0 });
	vp.Position({ 0,32 });
	vp.Source(L"udp://@192.168.18.113:11451");
	//vp.Source(L"D:/Videos/OBS Temporary/2023-06-04 17-44-42.mp4");

	OSDOverlay osdx(&vp);
	osdx.Coord(COORD_FILL, COORD_FILL);
	osdx.Size({ 0,0 });
	osd = &osdx;
	osdx.AddEventListener([](Message, WPARAM, LPARAM lParam)
		{
			camplt_mbpos.x = GET_X_LPARAM(lParam);
			camplt_mbpos.y = GET_Y_LPARAM(lParam);
			camplt_mbd = true;
		}, FE_LBUTTONDOWN);
	osdx.AddEventListener([](...)
		{
			camplt_mbd = false;

		}, FE_LBUTTONUP);
	auto campinvk = []()
	{
		CommandPack p;
		p.command = COMMAND_CAMERA;

		auto cam = configs[L"calibration"][L"Camera"];
		int pn = wcstol(cam[L"pitch_negative"].as_string().c_str(), nullptr, 10);
		int pp = wcstol(cam[L"pitch_positive"].as_string().c_str(), nullptr, 10);
		int pz = wcstol(cam[L"pitch_zero"].as_string().c_str(), nullptr, 10);
		int yn = wcstol(cam[L"yaw_negative"].as_string().c_str(), nullptr, 10);
		int yp = wcstol(cam[L"yaw_positive"].as_string().c_str(), nullptr, 10);
		int yz = wcstol(cam[L"yaw_zero"].as_string().c_str(), nullptr, 10);

		p.args[0] = CalibratedValue(yp, yn, yz, camplt_yaw);
		p.args[1] = CalibratedValue(pp, pn, pz, camplt_pitch);
		SendPacket(p, REMOTE_IP, 10485);
		//printf("camplt yaw: %d, pitch: %d\n", p.args[0], p.args[1]);

	};
	osdx.AddEventListener([&](Message, WPARAM, LPARAM lParam)
		{
			if (camplt_mbd)
			{
				auto x = GET_X_LPARAM(lParam);
				auto y = GET_Y_LPARAM(lParam);

				camplt_yaw += (x - camplt_mbpos.x) * camplt_sens;
				camplt_pitch -= (y - camplt_mbpos.y) * camplt_sens;

				camplt_mbpos.x = x;
				camplt_mbpos.y = y;
				if (camplt_yaw > 1) camplt_yaw = 1;
				if (camplt_yaw < -1) camplt_yaw = -1;
				if (camplt_pitch > 1) camplt_pitch = 1;
				if (camplt_pitch < -1) camplt_pitch = -1;
				campinvk();
			}
		}, FE_MOUSEMOVE);

	osdx.AddEventListener([&](...)
		{
			camplt_yaw = camplt_pitch = 0;
			campinvk();
		}, FE_RBUTTONDOWN);
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
		Button* btn = new Button(&mainFrame);
		btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		btn->Padding({ 10,5,10,5 });
		btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		btn->Position({ 10, 370 });
		new Label(btn, L"校准陀螺仪");
		btn->AddEventListener([](Message, WPARAM, LPARAM)
			{
				CommandPack p;
				p.command = COMMAND_MPU_CALIBRATE;
				p.args[0] = 1;
				SendPacket(p, REMOTE_IP, 10485);
			}, FE_LBUTTONDOWN);

		btn = new Button(&mainFrame);
		btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		btn->Padding({ 10,5,10,5 });
		btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		btn->Position({ 100, 370 });
		new Label(btn, L"校准磁力计");
		btn->AddEventListener([](Message, WPARAM, LPARAM)
			{
				CommandPack p;
				p.command = COMMAND_MPU_CALIBRATE;
				p.args[0] = 2;
				SendPacket(p, REMOTE_IP, 10485);
			}, FE_LBUTTONDOWN);

		TextEditor* teor, * teop;
		teop = new TextEditor(&mainFrame);
		teop->Content(L"0");
		teop->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		teop->Position({ 120, 410 });
		teop->Size({ 50, 26 });
		btn = new Button(&mainFrame);
		btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		btn->Padding({ 10,5,10,5 });
		btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		btn->Position({ 10, 410 });
		new Label(btn, L"设置俯仰偏移");
		btn->AddEventListener([teop](Message, WPARAM, LPARAM)
			{
				pitcho = std::stof(teop->Content());
			}, FE_LBUTTONDOWN);

		teor = new TextEditor(&mainFrame);
		teor->Content(L"0");
		teor->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		teor->Position({ 120, 450 });
		teor->Size({ 50, 26 });
		btn = new Button(&mainFrame);
		btn->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_CHILDREN);
		btn->Padding({ 10,5,10,5 });
		btn->Coord(COORD_NEGATIVE, COORD_POSITIVE);
		btn->Position({ 10, 450 });
		new Label(btn, L"设置横滚偏移");
		btn->AddEventListener([teor](Message, WPARAM, LPARAM)
			{
				rollo = std::stof(teor->Content());

			}, FE_LBUTTONDOWN);

	}
	//fbFrame.Show();
	ttframe = &fbFrame;
	//ShowInputCheckWindow();
	//ShowControlWindow();
	//ShowFilrecViewerWindow();


	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Controls, 0, 0, nullptr);
	crt.mouseable = false;
	crt.Alpha(1);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlRecv, (LPVOID)OnRecvTransmisson, 0, nullptr);
	//ShowDSPWindow();
	mainFrame.MainLoop();
	SaveConfigs();
	return 0;
}

void Amtr_Mav(float y, float p, float r)
{
	amtr_mav->SetYPR(y, p, r);
}

void OSDUpdateYPR(float y, float p, float r)
{
	osd->SetYPR(y, p, r);
}
void OSDUpdateGPS(float lat, float lon)
{
	osd->SetGPS(lat, lon);
}
void OSDUpdateSpeed(float as, float gs)
{
	osd->SetSpeeds(as, gs);
}
void OSDUpdateAlt(float alt, float climb)
{
	osd->SetAltc(alt, climb);
}

//normv: -1 ~ 1
int CalibratedValue(int maxv, int minv, int zerov, float normv)
{
	if (normv > 0)
	{
		return zerov + (maxv - zerov) * normv;
	}
	else if (normv < 0)
	{
		return zerov - (minv - zerov) * normv;
	}
	return zerov;

}