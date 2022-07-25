#pragma once
#include "memory.h"

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
		float x, y, z;
		float yaw, pitch, roll;
	} gyro;
	struct
	{
		float x, y, z;
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

		struct
		{
			float x, y, z;
		} velocity;
	} remote;
};
enum COMMAND : uint32_t
{
	COMMAND_MPU_RESET,		  // no parameters
	COMMAND_MPU_CALIBRATE,	  //[0]=0:both 1:mag 2:gyro
	COMMAND_REQUEST_CONFIG,	  //--
	COMMAND_AUTO_FLIGHT,	  //[0]=0:off 1:on
	COMMAND_RETURN,			  //--
	COMMAND_SELF_DESTRUCT,	  //--
	COMMAND_SELFCHECK_STATUS, //[0]=0:success 1:failed
	COMMAND_CAMERA			  //[0]=yaw out us [1]=pitch out us
};
struct MeetersPack
{
	struct
	{
		float voltage, current, temperature;
	} batteries[4];
	struct
	{
		float current, temperature, temp_conditioner;
	} motors[4];
	struct
	{
		float contorl, imgtx;
	} signal;
};
struct ConfigPack
{
	struct
	{
		uint8_t acclDynamic;
		uint8_t gyroDynamic;
		uint8_t acclDLPF;
		uint8_t gyroDLPF;
	} mpu;
	struct
	{
		struct
		{
			float w, x, y, z;
		} auttitide;
		struct
		{
			float offset;
			float range;
		} yaw;
		struct
		{
			float offset;
			float range;
		} pitch;
		struct
		{
			float offset;
			float range;
		} roll;

	} calibrations;
};
struct CommandPack
{
	COMMAND command;
	uint32_t args[16];
};
struct MavLinkPack
{
	struct
	{
		short len;
		bool tx;
	} meta;
	char data[384];
	bool TX()
	{
		return meta.tx;
	}
	bool RX()
	{
		return !TX();
	}
	void TX(bool v)
	{
		meta.tx = v;
	}
	void RX(bool v)
	{
		TX(!v);
	}
};
enum PACKET_TYPE : int32_t
{
	PACKET_TYPE_CONTROL,
	PACKET_TYPE_FEEDBACK,
	PACKET_TYPE_CONFIG,
	PACKET_TYPE_COMMAND,
	PACKET_TYPE_MEETERS,
	PACKET_TYPE_MAVLINK
};
struct Packet
{
	PACKET_TYPE type;
	union
	{
		ControlPack control;
		FeedbackPack feedback;
		ConfigPack config;
		CommandPack command;
		MeetersPack meeters;
		MavLinkPack mavlink;
	};

	Packet() { type = PACKET_TYPE_COMMAND; };
	Packet(PACKET_TYPE type) { this->type = type; };
	Packet(ControlPack& p)
	{
		type = PACKET_TYPE_CONTROL;
		control = p;
	}
	Packet(FeedbackPack& p)
	{
		type = PACKET_TYPE_FEEDBACK;
		feedback = p;
	}
	Packet(ConfigPack& p)
	{
		type = PACKET_TYPE_CONFIG;
		config = p;
	}
	Packet(CommandPack& p)
	{
		type = PACKET_TYPE_COMMAND;
		command = p;
	}
	Packet(MeetersPack& p)
	{
		type = PACKET_TYPE_MEETERS;
		meeters = p;
	}
	Packet(MavLinkPack& p)
	{
		type = PACKET_TYPE_MAVLINK;
		mavlink = p;
	}
};

inline int sizeofpack(Packet& p)
{
	auto t = p.type;
	switch (t)
	{
	case PACKET_TYPE_CONTROL:
		return sizeof(ControlPack) + 4;
	case PACKET_TYPE_FEEDBACK:
		return sizeof(FeedbackPack) + 4;
	case PACKET_TYPE_CONFIG:
		return sizeof(ConfigPack) + 4;
	case PACKET_TYPE_COMMAND:
		return sizeof(CommandPack) + 4;
	case PACKET_TYPE_MEETERS:
		return sizeof(MeetersPack) + 4;
	case PACKET_TYPE_MAVLINK:
		return p.mavlink.meta.len + sizeof(p.mavlink.meta) + 8;
	default:
		break;
	}
}
