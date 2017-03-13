#include <iostream>
using namespace std;

class A
{
public:
	virtual void func() = 0;
protected:
private:
};

void A::func() //error C2259: 'B' : cannot instantiate abstract class
{
	cout << "A::func()" << endl;
}

class B : public A
{
public:
protected:
private:
};

int main()
{
	B b;
	b.func();

	return 0;
}