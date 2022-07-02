#include "winsock.h"
#include "ControlPack.h"
#include <thread>

ControlPack input;

SOCKET sk_control = -1;
void InitTransmission()
{
	if (sk_control != -1) closesocket(sk_control);
	sk_control = socket(AF_INET, SOCK_DGRAM, 0);

	SOCKADDR_IN saddr;
	saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(6061);//¶Ë¿ÚºÅ
	bind(sk_control, (SOCKADDR*)&saddr, sizeof(SOCKADDR));
}
void SendControl(ControlPack cp)
{
	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(11401);
	sendto(sk_control, nullptr, 0, 0, (SOCKADDR*)&addr, sizeof addr);
	std::this_thread::sleep_for(std::chrono::milliseconds(8));
}