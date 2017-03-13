#include <iostream>
using namespace std;

class Base
{
public:
	virtual void Play()
	{
		cout << "Base::Play()" << endl;
	}
	void CallPlay()
	{
		this->Play();
	}
};

class Derived :  public Base
{
public:
	virtual void Play()
	{
		cout << "Derived::Play()" << endl;
	}
};

int main()
{
	Derived d;
	d.CallPlay();

	return 0;
}