#include "Manager.h"
#include <fstream>
#include <ctime>
#include <string>
using namespace std;
Manager::Manager()
{
	
}
Manager::Manager(wstring path)
{
	this->path = path;
	fstream fs;
	fs.open(path + L"\\book.fdb", ios::in | ios::binary);
	if (!fs)
	{
		fs.open(path + L"\\book.fdb", ios::out | ios::binary);
	}
	else
	{
		while (!fs.eof() && fs.peek() != EOF)
		{
			AMBookInfo(BookInfo(fs));
		}
	}
	fs.close();

	fs.open(path + L"\\instances.fdb", ios::in | ios::binary);
	if (!fs)
	{
		fs.open(path + L"\\instances.fdb", ios::out | ios::binary);
	}
	else
	{
		while (!fs.eof() && fs.peek() != EOF)
		{
			AMBookInstance(Book(fs));
		}
	}
	fs.close();

	fs.open(path + L"\\borrowers.fdb", ios::in | ios::binary);
	if (!fs)
	{
		fs.open(path + L"\\borrowers.fdb", ios::out | ios::binary);
	}
	else
	{
		while (!fs.eof() && fs.peek() != EOF)
		{
			AMBorrower(Borrower(fs));
		}
	}
	fs.close();

	Refresh();

}
void Manager::Refresh()
{
	for (auto i = borrowers.begin(); i != borrowers.end(); ++i)
	{
		i->second->lents.clear();
	}
	for (auto i = books.begin(); i != books.end(); ++i)
	{
		if (!i->second) continue;
		for (auto n = i->second->lents.begin(); n != i->second->lents.end(); ++n)
		{
			Borrower* br = borrowers[n->borrower];
			if (br)
			{
				br->AddLent(*i->second, (*n));
			}
		}
		i->second->_set_ref(bookinfo[i->second->infoIdentifier()]);
	}
}


Identifier Manager::AMBorrower(Borrower& b)
{
	borrowers[b.identifier()] = new Borrower(b);
	return b.identifier();
}
Identifier Manager::AMBookInfo(BookInfo& b)
{
	bookinfo[b.identifier()] = new BookInfo(b);
	return b.identifier();
}
Identifier Manager::AMBookInstance(Book& b)
{
	auto nb = books[b.identifier()] = new Book(b);
	bookinfo[nb->infoIdentifier()]->Instance(*nb);
	return nb->identifier();
}

LNTERR Manager::NewLent(Identifier book, LentInfo lent)
{
	Book* bk = books[book];
	Borrower* br = borrowers[lent.borrower];
	if (!bk) return LNTERR_BKNOEXIST;
	if (!br) return LNTERR_BRNOEXIST;

	for (auto i = bk->lents.begin(); i != bk->lents.end(); ++i)
	{
		if (i->returntime == 0)
			return LNTERR_NORET;
	}
	bk->lents.push_back(lent);
	br->AddLent(*bk, lent);
	Refresh();
	return LNTERR_OK;
}
RETERR Manager::ReturnLent(Identifier book, Identifier borrower)
{
	Book* bk = books[book];
	Borrower* br = borrowers[borrower];
	if (!bk || !br) return RETERR_NOEXIST;
	for (auto i = bk->lents.rbegin(); i != bk->lents.rend(); ++i)
	{
		if (i->borrower == borrower && i->returntime == 0)
		{
			i->returntime = time(0);
			if (i->returntime - i->lentout > i->limitation)
			{
				int bill = ((i->returntime - i->lentout) / 24 / 60 / 60) * bk->info()->priceOverduePd;
				if (br->balance - bill < 0)
					return RETERR_NOBALANCE;
				br->balance -= bill;
			}
			Refresh();
			return RETERR_OK;
		}
	}
	return RETERR_NOEXIST;
}
int Manager::GetBill(Identifier book, Identifier borrower)
{
	Book* bk = books[book];
	Borrower* br = borrowers[borrower];
	if (!bk || !br) return RETERR_NOEXIST;
	for (auto i = bk->lents.rbegin(); i != bk->lents.rend(); ++i)
	{
		if (i->borrower == borrower && i->returntime == 0)
		{
			i->returntime = time(0);
			if (i->returntime - i->lentout > i->limitation)
			{
				int bill = ((i->returntime - i->lentout) / 24 / 60 / 60) * bk->info()->priceOverduePd;
				i->returntime = 0;
				return bill;

			}
			i->returntime = 0;
			Refresh();
			return 0;
		}
	}
	return 0;

}

bool  Manager::RMBook(Identifier)
{
	return false;

}
bool  Manager::RMBookInstance(Identifier id)
{
	Book* bk = books[id];
	if (bk)
	{
		for (auto i = bk->lents.begin(); i != bk->lents.end(); ++i)
		{
			if (i->returntime == 0)
				return false;
		}
		bk->info()->Remove(id);
		books.erase(id);
	}
	return true;
}
bool  Manager::RMBorrower(Identifier)
{
	return false;

}

Book* Manager::GetBook(Identifier id)
{
	return books[id];
}
BookInfo* Manager::GetBookInfo(Identifier id)
{
	return bookinfo[id];
}
Borrower* Manager::GetBorrower(Identifier id)
{
	return borrowers[id];
}


vector<Book*> Manager::SelectInstance(wstring subs, KFILTER filter)
{
	vector<BookInfo*> info = SelectInfo(subs, filter);
	vector<Book*> res;
	for (auto i = info.begin(); i != info.end(); ++i)
	{
		auto v = (*i)->Instances();
		res.insert(res.end(), v.begin(), v.end());

	}
	return res;
}
vector<BookInfo*>  Manager::SelectInfo(wstring keyw, KFILTER filter)
{
	vector<BookInfo*> info;
	map<Identifier, BookInfo*>::iterator end;
	int count = 0;
	for (auto i = bookinfo.begin(); i != bookinfo.end(); ++i)
	{
		auto andd = filter & KFILTER_NAME;
		if ((filter & KFILTER_NAME) && i->second->name.find(keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else if ((filter & KFILTER_PUBLISHER) && i->second->publisher.find(keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else if ((filter & KFILTER_AUTHOR) && i->second->author.find(keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else if ((filter & KFILTER_CATAGORY) && i->second->catagory.find(keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else
		{
			if ((filter & KFILTER_ISBN))
			{
				wstring isbn;
				isbn.resize(13);
				for (int n = 0; n < 13; n++)
				{
					isbn[n] = (wchar_t)(i->second->ISBN[n]);
				}
				if (isbn.find(keyw) != wstring::npos)
				{
					info.push_back(i->second);
					break;
				}
			}
			if ((filter & KFILTER_IDENTIFIER))
			{
				wstring idstr = to_wstring(i->second->identifier());
				if (idstr.find(keyw) != wstring::npos)
				{
					info.push_back(i->second);
					break;
				}
				auto ks = i->second->Instances();
				for (auto n = ks.begin(); n != ks.end(); n++)
				{
					wstring idstr = to_wstring((*n)->identifier());
					if (idstr.find(keyw) != wstring::npos)
					{
						info.push_back(i->second);
						break;
					}
				}
			}
		}

	}
	return info;
}
vector<Borrower*> Manager::SelectBorrower(wstring subs, RFILTER filter)
{
	vector<Borrower*> res;
	for (auto i = borrowers.begin(); i != borrowers.end(); i++)
	{
		if ((filter & RFILTER_NAME) && i->second->name.find(subs) != wstring::npos)
		{
			res.push_back(i->second);
		}
		else if ((filter & RFILTER_PUBLICID) && i->second->publicIdentifier.find(subs) != wstring::npos)
		{
			res.push_back(i->second);
		}
		else
		{
			if ((filter & RFILTER_IDENTIFIER))
			{
				if (to_wstring(i->second->identifier()).find(subs) != wstring::npos)
				{
					res.push_back(i->second);
					break;
				}
			}
		}
	}

	return res;
}

Collection Manager::NextPage(Collection& current)
{
	if (!current.count) return {};
	vector<BookInfo*> info;
	vector<Book*> res;
	map<Identifier, BookInfo*>::iterator end;
	int count = 0;
	for (auto i = current.begin; i != bookinfo.end(); ++i)
	{
		auto andd = current.filter & KFILTER_NAME;
		if ((current.filter & KFILTER_NAME) && i->second->name.find(current.keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else if ((current.filter & KFILTER_PUBLISHER) && i->second->publisher.find(current.keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else if ((current.filter & KFILTER_AUTHOR) && i->second->author.find(current.keyw) != wstring::npos)
		{
			info.push_back(i->second);
		}
		else
		{
			if ((current.filter & KFILTER_ISBN))
			{
				wstring isbn;
				isbn.resize(13);
				for (int n = 0; n < 13; n++)
				{
					isbn[n] = (wchar_t)(i->second->ISBN[n]);
				}
				if (isbn.find(current.keyw) != wstring::npos)
				{
					info.push_back(i->second);
					goto skip;
				}
			}
			if ((current.filter & KFILTER_IDENTIFIER))
			{
				auto ks = i->second->Instances();
				for (auto n = ks.begin(); n != ks.end(); n++)
				{
					wstring idstr = to_wstring((*n)->identifier());
					if (idstr.find(current.keyw) != wstring::npos)
					{
						info.push_back(i->second);
						goto skip;
					}
				}
			}
		}
	skip:
		if (info.size() >= current.count)
		{
			end = ++i;
			count = current.count;
			break;
		}
	}
	for (auto i = info.begin(); i != info.end(); ++i)
	{
		auto v = (*i)->Instances();
		res.insert(res.end(), v.begin(), v.end());

	}
	return { res, current.filter,count,current.keyw, end };
}

#include   <algorithm> 
vector<Record>  Manager::SelectRecord(wstring keyw)
{
	vector<Record> ret;
	for (auto bk = books.begin(); bk != books.end(); bk++)
	{
		if (bk->second)
		{
			auto lents = bk->second->lents;
			for (auto l = lents.begin(); l != lents.end(); ++l)
			{
				Borrower* br = GetBorrower(l->borrower);
				if (br)
				{
					if (bk->second->info()->name.find(keyw) != wstring::npos)
						ret.push_back({ bk->second ,*l });
					else if (bk->second->info()->GetISBN().find(keyw) != wstring::npos)
						ret.push_back({ bk->second ,*l });
					else if (br->name.find(keyw) != wstring::npos)
						ret.push_back({ bk->second ,*l });
					else if (br->publicIdentifier.find(keyw) != wstring::npos)
						ret.push_back({ bk->second ,*l });

				}
			}
		}
	}
	sort(ret.begin(), ret.end());
	return ret;
}

void Manager::Commit()
{
	ofstream fs(path + L"\\book.fdb", ios::binary);
	for (auto i = bookinfo.begin(); i != bookinfo.end(); ++i)
	{
		if (i->second)
			i->second->Serialize(fs);
	}
	fs.close();

	fs.open(path + L"\\instances.fdb", ios::binary);
	for (auto i = books.begin(); i != books.end(); ++i)
	{
		if (i->second)
			i->second->Serialize(fs);
	}
	fs.close();

	fs.open(path + L"\\borrowers.fdb", ios::binary);
	for (auto i = borrowers.begin(); i != borrowers.end(); ++i)
	{
		if (i->second)
			i->second->Serialize(fs);
	}
	fs.close();
}


