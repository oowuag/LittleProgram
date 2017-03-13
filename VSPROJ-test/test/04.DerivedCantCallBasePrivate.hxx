#include <iostream>
using namespace std;

class A
{
public:
	virtual void foobar(int &a);
private:
	int m_a;
};

void A::foobar(int &a)
{
	cout << a << endl;
}

class B : public A
{
public:
	void foobarB()
	{
		cout << m_a << endl; //error
	}
};

int main()
{
	B b;
	b.foobarB();


	return 0;
}