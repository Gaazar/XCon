#include "Transmission.h"
#include "Winsock2.h"
#include <thread>
#include <string>
#include <Ws2tcpip.h>
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

void SendPacket(Packet p, std::string raddr, unsigned short port)
{
	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr(raddr.c_str());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	sendto(sk_control, (char*)&p, sizeofpack(p), 0, (SOCKADDR*)&addr, sizeof addr);
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
SOCKADDR_IN addr4;
SOCKADDR_IN a4;
sockaddr_in6 addr6;
sockaddr_in6 a6;

SOCKET sk4;
SOCKET sk6;

void* mavpx(void*)
{
	while (true)
	{
		sockaddr_in r4;
		int rlen = sizeof(r4);
		char buffer[512];
		int len = recvfrom(sk4, buffer, sizeof buffer, 0, (SOCKADDR*)&r4, &rlen);
		if (len > 0)
		{
			sendto(sk6, buffer, len, 0, (sockaddr*)&a6, sizeof a6);
		}

	}

}

void mavpxt()
{
	sk6 = socket(AF_INET6, SOCK_DGRAM, 0);
	sk4 = socket(AF_INET, SOCK_DGRAM, 0);

	addr4.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr4.sin_family = AF_INET;
	addr4.sin_port = htons(20220);//¶Ë¿ÚºÅ
	bind(sk4, (SOCKADDR*)&addr4, sizeof(addr4));
	auto bret = ::bind(sk4, (SOCKADDR*)&addr4, sizeof(addr4));

	a4.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	a4.sin_family = AF_INET;
	a4.sin_port = htons(20222);//¶Ë¿ÚºÅ


	inet_pton(AF_INET6, "2409:8a5c:2216:6840:7caa:8165:7f06:510f", &addr6.sin6_addr);
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(20221);//¶Ë¿ÚºÅ
	bret = ::bind(sk6, (SOCKADDR*)&addr6, sizeof(addr6));

	inet_pton(AF_INET6, "2409:8a5c:2217:9df0:5427:6829:81bf:2fab", &a6.sin6_addr);
	a6.sin6_family = AF_INET6;
	a6.sin6_port = htons(20222);//¶Ë¿ÚºÅ

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mavpx, 0, 0, nullptr);
	while (true)
	{
		sockaddr_in6 r6;
		int rlen = sizeof(r6);
		char buffer[512];
		int len = recvfrom(sk6, buffer, sizeof buffer, 0, (SOCKADDR*)&r6, &rlen);
		if (len > 0)
		{
			sendto(sk4, buffer, len, 0, (sockaddr*)&a4, sizeof a4);

		}

	}

}