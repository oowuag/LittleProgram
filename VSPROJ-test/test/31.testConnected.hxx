#include <iostream>
#include <Windows.h>
using namespace std;

typedef char NCCHAR;

class NEStatusListenerIF
{
public:
virtual void onConnected(const NCCHAR* target) = 0;
virtual void onDisconnected(const NCCHAR* target) = 0;
};


class NCHandlerBase
{
public:
	NCHandlerBase()
		:eventListener(*this)
	{
		cout << "NCHandlerBase::NCHandlerBase" << endl;
	}
protected:
    virtual VOID OnConnected(const NCCHAR* strClientName);
    virtual VOID OnDisconnected(const NCCHAR* strClientName);
protected:
	class Listener :public NEStatusListenerIF 
	{
	public:
		Listener(NCHandlerBase& nh):handler(nh) {}
		~Listener() {}

		virtual void onConnected(const NCCHAR* target)
		{
			handler.OnConnected(target);
		}
		virtual void onDisconnected(const NCCHAR* target)
		{
			handler.OnDisconnected(target);
		}
	private:
		NCHandlerBase& handler;
	} eventListener;

public:
	void callonConnected()
	{
		eventListener.onConnected(NULL);

	}

};

VOID NCHandlerBase::OnConnected(const NCCHAR* strClientName)
{
	cout << "NCHandlerBase::OnConnected" << endl;
}
VOID NCHandlerBase::OnDisconnected(const NCCHAR* strClientName)
{
	cout << "NCHandlerBase::OnDisconnected" << endl;
}

class NCSnsHandler : public NCHandlerBase
{
public:
private:
	VOID OnConnected(const NCCHAR* strClientName)
	{
		cout << "NCSnsHandler::OnConnected" << endl;
	}
	VOID OnDisconnected(const NCCHAR* strClientName)
	{
		cout << "NCSnsHandler::OnDisconnected" << endl;
	}
};

int CMatrix[2+1][3+2];

int main()
{
	cout << sizeof(CMatrix) << endl;
	//NCSnsHandler snshandler;
	//snshandler.callonConnected();
	unsigned int x = -65500;
	if (x > 0)
	{
		cout << "OK" << endl;
	}
	printf("%lu", x);


	return 0;
}

