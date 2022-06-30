#pragma once

#include <istream>
#include <ostream>
#include <iostream>
#include <string>
#include <vector>

#define PDB_BOOK "F:\\Learn\\FlameUI\\db\\book.fdb"
#define PDB_BRWR "F:\\Learn\\FlameUI\\db\\borrower.fdb"

typedef unsigned long long Identifier;
typedef unsigned int prefix;

template<typename T>
size_t Write(std::ostream& s, T& dat)
{
	s.write((char*)&dat, sizeof(T));
	return sizeof(T);
}

template<typename T>
size_t Write(std::ostream& s, std::vector<T>& vec)
{
	size_t ttlsz = 0;
	ttlsz += WriteConst(s, (prefix)vec.size());
	for (auto i = vec.begin(); i != vec.end(); ++i)
	{
		ttlsz += Write(s, *i);
	}
	return ttlsz;
}

template<typename T>
size_t Read(std::istream& s, std::vector<T>& vec)
{
	size_t ttlsz = 0;
	prefix cnt;
	ttlsz += Read(s, cnt);
	vec.resize(cnt);
	ttlsz += cnt * sizeof(T);
	for (auto i = 0; i < cnt; ++i)
	{
		Read(s, vec[i]);
	}
	return ttlsz;
}


template<typename T>
size_t WriteConst(std::ostream& s, T dat)
{
	s.write((char*)&dat, sizeof(T));
	return sizeof(T);
}
extern size_t Write(std::ostream& s, std::wstring& dat);
extern size_t Read(std::istream& s, std::wstring& dat);


template<typename T>
size_t Read(std::istream& s, T& dat)
{
	s.read((char*)&dat, sizeof(T));
	return sizeof(T);
}


class ISerializer
{
public:
	virtual size_t Serialize(std::ostream& s) = 0;
	virtual size_t Deserialize(std::istream& s) = 0;

};

Identifier GenUniqueIdentifier();
