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

class Msglnfo  //���class ����������Ϣ���Ա�����������Ϣ
{ 
};

template<typename Company>
class MsgSender
{
public:
	//���캯�������������ȵȡ�
	void sendClear(const Msglnfo& info)
	{
		std::string msg;
		//�����������info ������Ϣ
		Company c;
		c.sendCleartext(msg);
	}
	void sendSecret(const Msglnfo& info) //����sendClear��Ψһ��ͬ��
	{
		//�������c. sendEncrypted
	} 
};

template<typename Company>
class LoggingMsgSender: public MsgSender<Company>
{
public:
	//using MsgSender<Company>::sendClear;
	//���캯�������������ȵ�.
	void sendClearMsg(const Msglnfo& info)
	{
		//��"����ǰ"����Ϣд��log;
		sendClear(info); //����base class ����:������޷�ͨ�����롣
						//g++  ���벻��
//eff1.cxx:60: error: there are no arguments to `sendClear' that depend on a template parameter, 
//so a declaration of `sendClear' must be available

		//��"���ͺ�"����Ϣд��log;
	}
};

int main()
{
	Msglnfo msfInfo;
	//MsgSender<CompanyA> msgsendA;
	//msgsendA.sendClear(msfInfo);


	LoggingMsgSender<CompanyA> logmsgsendA;
	logmsgsendA.sendClearMsg(msfInfo);

	LoggingMsgSender<CompanyZ> logmsgsendZ; //���뻹�ǻ����
	logmsgsendZ.sendClearMsg(msfInfo);


	return 0;
}
