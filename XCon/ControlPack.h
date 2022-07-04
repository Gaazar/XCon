#pragma once

struct ControlPack
{
	short accelerator;
	short yaw;
	short pitch;
	short roll;
};

struct FeedbackPack
{
	unsigned int id;
	struct
	{
		short x, y, z;
		short yaw, picth, roll;
	} gyro;
	struct
	{
		short x, y, z;
	} magnetometer;
	struct
	{
		float longtitude, latitude, altitude;
	} gps;
	struct
	{
		struct
		{
			float x, y, z;
		} position;

		struct
		{
			float x, y, z, w;
		} attitude;

		struct {
			float x, y, z;
		} velocity;
	} remote;

};
