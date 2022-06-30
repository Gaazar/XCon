#pragma once
#include "ISerializer.h"
#include <string>
#include <vector>
#include <map>

using namespace std;
typedef unsigned short Catagory;
class Borrower;
class Book;

class BookInfo :ISerializer
{
private:
	Identifier internalIdentifier;
	map<Identifier, Book*> instances;
	streamoff offset;
public:
	BookInfo(const char* ISBN, wstring cat, wstring name, wstring pubs, wstring auth, unsigned int price = 1000);
	BookInfo(istream& s);
	BookInfo(const BookInfo& b);
	char ISBN[13];
	wstring catagory;
	wstring name;
	wstring publisher;
	wstring author;
	int price;//in cent || 0.01 yuan
	time_t lentLimitation = 30 * 24 * 60 * 60;
	int priceOverduePd = 10;
	void Remove(Identifier);
	wstring GetISBN()
	{
		wstring isbn;
		isbn.resize(13);
		for (int n = 0; n < 13; n++)
		{
			isbn[n] = (wchar_t)(ISBN[n]);
		}
		return isbn;
	}
	Identifier identifier() const;
	size_t Serialize(std::ostream& s) override;
	size_t Deserialize(std::istream& s) override;
	void Instance(Book& book);
	Book* Instance(Identifier id);
	vector<Book*> Instances() const;

};

struct LentInfo
{
	Identifier borrower;
	time_t lentout; //time stamp
	time_t limitation = 30 * 24 * 60 * 60; //time stamp
	time_t returntime; //time stamp
};

class Book :ISerializer
{
private:
	Identifier uniqueIdentifier;
	BookInfo* infoRef;
	Identifier bookInfo;
	streamoff offset;
public:
	vector<LentInfo> lents;

	Book(BookInfo& info);
	Book(std::istream& s);
	Book(const Book& b);
	Book(Identifier info);
	void _set_ref(BookInfo* bi)
	{
		infoRef = bi;
	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name=""></param>
	/// <param name="limitaion">in second</param>
	void AddLent(Borrower& borrower, time_t limitation);
	Identifier identifier() const;
	Identifier infoIdentifier() const;
	BookInfo* info() const;
	size_t Serialize(std::ostream& s) override;
	size_t Deserialize(std::istream& s) override;
};

