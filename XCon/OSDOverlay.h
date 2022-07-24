#pragma once
#include "View.h"
using namespace FlameUI;
class OSDOverlay :
	public View
{
protected:
	LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
	void Draw() override;
private:
	void Animation(float, int, int) override;
	animation_id aid;
	float yaw_d, pitch_d, roll_d;
	float climb_d, alt_d, lon_d, lat_d;
	float aspeed_d, gspeed_d;
	float accthr_d;

	struct
	{
		float yaw, pitch, roll;
		float climb, alt, lon, lat;
		float aspeed, gspeed;
		float accthr;
	} rawdata;

	ID2D1PathGeometry* rollGeom;
	IDWriteTextFormat* fmt;
public:
	struct
	{
		float degs_f = 0.2f;
		float degs_s = 0.05f;
		float meter_f = 0.2f;
		float meter_s = 0.1f;
	}
	smooth;
	OSDOverlay(View* parent);
	void SetYPR(float y, float p, float r);
	void SetGPS(float lat, float lon);
	void SetSpeeds(float as, float gs);
	void SetAltc(float alt, float climb);

};

