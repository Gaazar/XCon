#include "Transmission.h"
#include "Winsock2.h"
#include <thread>
#include <string>
#pragma comment(lib,"ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable:4996)

ControlPack input;

SOCKET sk_control = -1;
bool started = false;
void InitTransmission(unsigned short port)
{
	if (!started)
	{
		DWORD ctid;
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(sockVersion, &data) != 0)
		{
			abort();
		}
		started = true;
	}
	if (sk_control != -1) closesocket(sk_control);
	sk_control = socket(AF_INET, SOCK_DGRAM, 0);

	SOCKADDR_IN saddr;
	saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);//¶Ë¿ÚºÅ
	bind(sk_control, (SOCKADDR*)&saddr, sizeof(SOCKADDR));
}
void CleanTransmission()
{
	closesocket(sk_control);
	WSACleanup();

}
void SendControl(ControlPack cp, std::string raddr, unsigned short port)
{
	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr(raddr.c_str());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	sendto(sk_control, (char*)&cp, sizeof cp, 0, (SOCKADDR*)&addr, sizeof addr);


}
void SendFeedback(FeedbackPack pk, std::string raddr, unsigned short port)
{
	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr(raddr.c_str());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	sendto(sk_control, (char*)&pk, sizeof pk, 0, (SOCKADDR*)&addr, sizeof addr);
}
void ControlRecv(void(*cb)(int, char*))
{
	SOCKADDR_IN remote;
	char buffer[1536];
	int rlen = sizeof(remote);
	while (true)
	{
		int len = recvfrom(sk_control, buffer, sizeof buffer, 0, (SOCKADDR*)&remote, &rlen);
		if (len > 0)
		{
			cb(len, buffer);
		}
	}
}