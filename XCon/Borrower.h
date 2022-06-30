#pragma once
#include "ISerializer.h"
#include <string>
#include <vector>

using namespace std;

struct LentInfo;
class Book;
enum SEX
{
	SEX_FEMALE,
	SEX_MALE
};
struct LB
{
		LentInfo* lent;
		Book* book;
};
class Borrower :ISerializer
{
private:

	Identifier internalIdentifier;
	streamoff offset;
public:
	Borrower(wstring name, time_t birthday, wstring id, SEX sex);
	Borrower(std::istream& s);
	Borrower(const Borrower& b);
	wstring name;
	time_t born;
	wstring publicIdentifier;
	SEX sex;
	vector<LB> lents;
	int balance;

	void AddLent(Book& b, LentInfo& l);

	Identifier identifier() const;
	size_t Serialize(std::ostream& s) override;
	size_t Deserialize(std::istream& s) override;

};