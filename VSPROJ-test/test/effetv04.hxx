//p.26.

#include <iostream>
using namespace std;

class Base1
{
public:
	Base1():b1(1) {}
	int b1;
};
class Base2
{
public:
	Base2():b2(2) {}
	int b2;
};
class Derived: public Base1, public Base2
{  
public:
	Derived():d(3) {}
	int d;
};


int main()
{
	Derived d;
	Base2* pb = &d; //隐喻地将Derived* 转换为Base*

	cout << (int*)(pb) << endl; //different
	cout << (int*)&d << endl;

	return 0;
}
