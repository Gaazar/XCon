#pragma once

#include <string>
#include <vector>

#include "Book.h"
#include "Borrower.h"

enum KFILTER
{
	KFILTER_IDENTIFIER = 1,
	KFILTER_NAME = 2,
	KFILTER_AUTHOR = 4,
	KFILTER_PUBLISHER = 8,
	KFILTER_ISBN = 16,
	KFILTER_CATAGORY = 32
};
enum RFILTER
{
	RFILTER_IDENTIFIER = 1,
	RFILTER_PUBLICID = 2,
	RFILTER_NAME = 4,
	RFILTER_SEX = 8,
	RFILTER_BIRTHDAY = 16,
};

enum RETERR
{
	RETERR_OK = 0,
	RETERR_NOBALANCE = 1,
	RETERR_NOEXIST = 2
};

enum LNTERR
{
	LNTERR_OK,
	LNTERR_BRNOEXIST,
	LNTERR_BKNOEXIST,
	LNTERR_MAXLENT,
	LNTERR_NORET
};
#define KFILTER_ALL ((KFILTER)(63))
#define RFILTER_ALL ((RFILTER)(31))
struct Collection
{
	std::vector<Book*> res;
	KFILTER filter;
	int count;
	wstring keyw;
	map<Identifier, BookInfo*>::iterator begin;
};
struct Record
{

	Book* book;
	LentInfo lent;

	bool operator < (const Record& rec) const //ÉýÐòÅÅÐòÊ±±ØÐëÐ´µÄº¯Êý
	{
		time_t a = lent.returntime, b = rec.lent.returntime;
		if (a == 0) a = 0xEFFFFFFFFF;
		if (b == 0) b = 0xEFFFFFFFFF;
		return  a > b;
	}
};
class Manager
{
private:
	std::wstring path;
	map<Identifier, BookInfo*> bookinfo;
	map<Identifier, Book*> books;
	map<Identifier, Borrower*> borrowers;

	void Refresh();
public:
	Manager(std::wstring path);
	Manager();
	/// <summary>
	/// AM = Add | Modify
	/// </summary>
	/// <param name="borrower"></param>
	Identifier AMBorrower(Borrower&);
	Identifier AMBookInfo(BookInfo&);
	Identifier AMBookInstance(Book&);

	LNTERR NewLent(Identifier book, LentInfo lent);
	RETERR ReturnLent(Identifier book, Identifier borrower);
	int GetBill(Identifier book, Identifier borrower);
	bool RMBook(Identifier);
	bool RMBookInstance(Identifier);
	bool RMBorrower(Identifier);

	Book* GetBook(Identifier);
	BookInfo* GetBookInfo(Identifier);
	Borrower* GetBorrower(Identifier);

	vector<Book*> SelectInstance(wstring subs, KFILTER filter = KFILTER_ALL);
	vector<BookInfo*> SelectInfo(wstring subs, KFILTER filter = KFILTER_ALL);
	vector<Borrower*> SelectBorrower(wstring subs, RFILTER filter = RFILTER_ALL);
	vector<Record> SelectRecord(wstring keyw);
	Collection NextPage(Collection& current);

	void Commit();

};

