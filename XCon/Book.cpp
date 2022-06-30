#include "Book.h"
#include "Borrower.h"
#include <ctime>
using namespace std;

BookInfo::BookInfo(const char* ISBN, wstring cat, wstring name, wstring pubs, wstring auth, unsigned int price)
{
	memcpy(&this->ISBN, ISBN, sizeof(this->ISBN));
	catagory = cat;
	this->name = name;
	publisher = pubs;
	author = auth;
	this->price = price;
	internalIdentifier = GenUniqueIdentifier();
}
BookInfo::BookInfo(istream& s)
{
	Deserialize(s);
}


BookInfo::BookInfo(const BookInfo& b)
{
	new (this)BookInfo(b.ISBN, b.catagory, b.name, b.publisher, b.author, b.price);
	internalIdentifier = b.internalIdentifier;
	instances = b.instances;
	lentLimitation = b.lentLimitation;
	priceOverduePd = b.priceOverduePd;
}
void BookInfo::Remove(Identifier id)
{
	instances.erase(id);
}
void BookInfo::Instance(Book& book)
{
	instances[book.identifier()] = &book;
}
Book* BookInfo::Instance(Identifier id)
{
	return instances[id];
}
Identifier BookInfo::identifier() const
{
	return internalIdentifier;
}
vector<Book*> BookInfo::Instances() const
{
	vector<Book*> res;
	for (auto i = instances.begin(); i != instances.end(); ++i)
	{
		if (i->second)
		{
			res.push_back(i->second);
		}
	}
	return res;
}



Book::Book(BookInfo& info)
{
	infoRef = &info;
	bookInfo = info.identifier();
	uniqueIdentifier = GenUniqueIdentifier();
	info.Instance(*this);
}
Book::Book(istream& s)
{
	Deserialize(s);
}
Book::Book(const Book& b)
{
	bookInfo = b.bookInfo;
	infoRef = b.infoRef;
	offset = b.offset;
	uniqueIdentifier = b.uniqueIdentifier;
	lents = b.lents;
}
Book::Book(Identifier info)
{
	bookInfo = info;
	uniqueIdentifier = GenUniqueIdentifier();
}
void Book::AddLent(Borrower& borrower, time_t limitation)
{
	LentInfo l;
	l.borrower = borrower.identifier();
	l.lentout = time(0);
	l.limitation = l.lentout + limitation;
	l.returntime = 0;
	lents.push_back(l);
}


Identifier Book::identifier() const
{
	return uniqueIdentifier;
}
Identifier  Book::infoIdentifier() const
{
	return bookInfo;
}
BookInfo* Book::info() const
{
	return infoRef;
}





size_t Book::Serialize(ostream& s)
{
	size_t ttlsz = 0;
	offset = s.tellp();
	ttlsz += Write(s, uniqueIdentifier);
	ttlsz += Write(s, bookInfo);
	ttlsz += Write(s, lents);
	return ttlsz;
}

size_t Book::Deserialize(istream& s)
{
	size_t ttlsz = 0;
	offset = s.tellg();
	ttlsz += Read(s, uniqueIdentifier);
	ttlsz += Read(s, bookInfo);
	ttlsz += Read(s, lents);
	return ttlsz;
}



size_t BookInfo::Serialize(ostream& s)
{
	size_t ttlsz = 0;
	offset = s.tellp();

	ttlsz += Write(s, internalIdentifier);
	ttlsz += Write(s, ISBN);
	ttlsz += Write(s, catagory);
	ttlsz += Write(s, name);
	ttlsz += Write(s, publisher);
	ttlsz += Write(s, author);
	ttlsz += Write(s, price);
	prefix cnt = instances.size();
	ttlsz += Write(s, cnt);
	for (auto i = instances.begin(); i != instances.end(); ++i)
	{
		ttlsz += Write(s, i->first);
	}

	return ttlsz;
}

size_t BookInfo::Deserialize(istream& s)
{
	size_t ttlsz = 0;
	offset = s.tellg();
	ttlsz += Read(s, internalIdentifier);
	ttlsz += Read(s, ISBN);
	ttlsz += Read(s, catagory);
	ttlsz += Read(s, name);
	ttlsz += Read(s, publisher);
	ttlsz += Read(s, author);
	ttlsz += Read(s, price);
	prefix cnt;
	Identifier idf;
	ttlsz += Read(s, cnt);
	for (int i = 0; i < cnt; i++)
	{
		ttlsz += Read(s, idf);
		instances.insert(pair<Identifier, Book*>(idf, nullptr));
	}

	return ttlsz;
}