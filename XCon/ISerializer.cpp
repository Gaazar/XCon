#include "ISerializer.h"
#include <chrono>

size_t Write(std::ostream& s, std::wstring& dat)
{
	prefix len = (prefix)dat.length() * 2;
	s.write((char*)&len, sizeof(len));
	if (len > 0)
	{
		s.write((char*)&dat[0], len);
	}
	return len + sizeof(len);
}

size_t Read(std::istream& s, std::wstring& dat)
{
	prefix len = 0;
	s.read((char*)&len, sizeof(len));
	if (len > 0)
	{
		dat.resize(len / 2);
		s.read((char*)&dat[0], len);
	}

	return len + sizeof(len);
}

Identifier GenUniqueIdentifier()
{
	static unsigned short ccnt;
	ccnt++;
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		);
	return (ms.count() << 4) + (ccnt & 0xF);
}