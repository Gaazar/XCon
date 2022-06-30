#include "Borrower.h"
#include "Book.h"
using namespace std;

Borrower::Borrower(wstring name, time_t birthday, wstring id, SEX sex)
{
	this->name = name;
	this->born = birthday;
	this->publicIdentifier = id;
	this->sex = sex;
	this->internalIdentifier = GenUniqueIdentifier();
}
Borrower::Borrower(istream& s)
{
	Deserialize(s);
}
Borrower::Borrower(const Borrower& b)
{
	name = b.name;
	born = b.born;
	publicIdentifier = b.publicIdentifier;
	sex = b.sex;
	internalIdentifier = b.internalIdentifier;
	balance = b.balance;
}

void Borrower::AddLent(Book& b, LentInfo& l)
{
	lents.push_back({ &l, &b });
}

Identifier Borrower::identifier() const
{
	return internalIdentifier;
}

size_t Borrower::Serialize(ostream& s)
{
	size_t ttlsz = 0;
	offset = s.tellp();
	ttlsz += Write(s, internalIdentifier);
	ttlsz += Write(s, name);
	ttlsz += Write(s, born);
	ttlsz += Write(s, publicIdentifier);
	ttlsz += Write(s, sex);
	ttlsz += Write(s, balance);
	return ttlsz;
}

size_t Borrower::Deserialize(istream& s)
{
	size_t ttlsz = 0;
	offset = s.tellg();
	ttlsz += Read(s, internalIdentifier);
	ttlsz += Read(s, name);
	ttlsz += Read(s, born);
	ttlsz += Read(s, publicIdentifier);
	ttlsz += Read(s, sex);
	ttlsz += Read(s, balance);
	return ttlsz;
}