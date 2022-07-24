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

	ID2D1PathGeometry* rollGeom;
	IDWriteTextFormat* fmt;
public:
	OSDOverlay(View* parent);

};

