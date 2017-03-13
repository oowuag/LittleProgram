#include <iostream>
using namespace std;

class Base
{
public:
	virtual void Play(int x)
	{
		cout << "Base::Play()" << x << endl;
	}
	void CallPlay()
	{
		this->Play(3);
	}
};

typedef void (Base::*ThisFunc)(int);
void Invoke(Base *pClass, ThisFunc funcptr, int x) 
{
	(pClass->*funcptr)(x); 
};

int main()
{
	Base d;
	d.CallPlay();

	//1.
	Invoke(&d, &Base::Play, 4);

	//2.
	ThisFunc pFunc2 = &Base::Play;
	(d.*pFunc2)(5);

	//成员函数指针
	//http://topic.csdn.net/t/20050525/11/4033949.html

	return 0;
}