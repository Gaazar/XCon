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
			default:
				break;
			}
			return;
		}
	}

}