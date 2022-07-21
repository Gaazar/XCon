#include "mavlink/common/mavlink.h"
#include "mavlink/protocol.h"
#include "ControlPack.h"
#include "stdio.h"

#define RAD2DEG (57.29577951)
void Amtr_Mav(float y, float p, float r);
void RxData(char* data, int len)
{
	mavlink_message_t msg;
	mavlink_status_t status;
	for (int i = 0; i < len; i++)
	{
		if (mavlink_parse_char(MAVLINK_COMM_0, data[i], &msg, &status))
		{
			//printf("msgid:%d len:%d\n", msg.msgid, msg.len);
			switch (msg.msgid)
			{
			case MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT:
			{
				mavlink_attitude_t a;
				mavlink_msg_attitude_decode(&msg, &a);
				if (a.yaw < 0) a.yaw += 2 * 3.1415926535;
				Amtr_Mav(a.yaw * RAD2DEG, a.pitch * RAD2DEG, a.roll * RAD2DEG);
				//printf("roll: %fdeg yaw: %fdeg pitch: %fdeg\n", a.roll * RAD2DEG, a.yaw * RAD2DEG, a.pitch * RAD2DEG);
				break;
			}
			case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
			{
				mavlink_global_position_int_t gp;
				mavlink_msg_global_position_int_decode(&msg, &gp);
				//printf("lat:%f lon:%f alt:%f\n", gp.lat / 10000000.f, gp.lon / 10000000.f, gp.alt / 1000.f);
				break;
			}
			case MAVLINK_MSG_ID_VFR_HUD:
			{
				mavlink_vfr_hud_t m;
				mavlink_msg_vfr_hud_decode(&msg, &m);
				printf("as:%f gs:%f climb:%f alt:%f head:%d \n", m.airspeed, m.groundspeed, m.climb, m.alt, m.heading);
			}
			default:
				break;
			}
			return;
		}
	}

}