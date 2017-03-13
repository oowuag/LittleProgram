#include <iostream>
using namespace std;

class CompanyA
{
public:
	void sendCleartext(const std::string& msg)
	{
		cout << "CompanyA::sendCleartext" << endl;
	}
	void sendEncrypted(const std::string& msg)
	{
		cout << "CompanyA::sendEncrypted" << endl;
	}
};

class CompanyB
{
public:
	void sendCleartext(const std::string& msg)
	{
		cout << "CompanyB::sendCleartext" << endl;
	}
	void sendEncrypted(const std::string& msg)
	{
		cout << "CompanyB::sendEncrypted" << endl;
	}
};

class CompanyZ
{
public:
};

class Msglnfo  //这个class 用来保存信息，以备将来产生信息
{ 
};

template<typename Company>
class MsgSender
{
public:
	//构造函数、析构函数等等。
	void sendClear(const Msglnfo& info)
	{
		std::string msg;
		//在这儿，根据info 产生信息
		Company c;
		c.sendCleartext(msg);
	}
	void sendSecret(const Msglnfo& info) //类似sendClear，唯一不同是
	{
		//这里调用c. sendEncrypted
	} 
};

template<typename Company>
class LoggingMsgSender: public MsgSender<Company>
{
public:
	//using MsgSender<Company>::sendClear;
	//构造函数、析构函数等等.
	void sendClearMsg(const Msglnfo& info)
	{
		//将"传送前"的信息写至log;
		sendClear(info); //调用base class 函数:这段码无法通过编译。
						//g++  编译不过
//eff1.cxx:60: error: there are no arguments to `sendClear' that depend on a template parameter, 
//so a declaration of `sendClear' must be available

		//将"传送后"的信息写至log;
	}
};

int main()
{
	Msglnfo msfInfo;
	//MsgSender<CompanyA> msgsendA;
	//msgsendA.sendClear(msfInfo);


	LoggingMsgSender<CompanyA> logmsgsendA;
	logmsgsendA.sendClearMsg(msfInfo);

	LoggingMsgSender<CompanyZ> logmsgsendZ; //编译还是会出错
	logmsgsendZ.sendClearMsg(msfInfo);


	return 0;
}
