#include <iostream>
using namespace std;

class IPerson { //这个class 指出需实现的接口
public:
	virtual ~IPerson() {}
	virtual std::string name() const = 0;
	virtual std::string birthDate() const = 0;
};

class DatabaseID { 
};

class PersonInfo {
public:
	explicit PersonInfo(DatabaseID pid) {}
	virtual ~PersonInfo() {}
	virtual const char* theName() const { return "theName"; }
	virtual const char* theBirthDate( ) const { return "theBirthDate"; }
	virtual const char* valueDelimOpen() const { return "valueDelimOpen"; }
	virtual const char* valueDelimClose() const { return "valueDelimClose"; }
};

class CPerson: public IPerson, private PersonInfo { //注意，多重继承
public:
	explicit CPerson(DatabaseID pid): PersonInfo(pid) { }
	virtual std::string name() const // IPerson member func
	{ return PersonInfo::theName(); }
	virtual std::string birthDate() const // IPerson member func
	{ return PersonInfo::theBirthDate(); }
private:
	const char* valueDelimOpen() const { return ""; }
	const char* valueDelimClose() const { return ""; }
};


int main()
{
	DatabaseID did;
	CPerson personA(did);


	return 0;
}