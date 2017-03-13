#include <iostream>  
using namespace std;  


class Base {
private:
	int x;
public:
	virtual void mf1()=0 { }
	virtual void mf1(int) { cout << "B:mf1(int)" << endl; }
	virtual void mf2() { cout << "B:mf2()" << endl; }
	void mf3 () { cout << "B:mf3()" << endl; }
	void mf3(double) { cout << "B:mf3(double)" << endl; }
};

class Derived: public Base {
public:
	using Base::mf1; //让Base class 内名为mf1 和mf3的所有东西
	using Base::mf3; //在Derived 作用域内都可见(并且public)

	virtual void mf1() { cout << "D:mf1()" << endl; }
	void mf3 () { cout << "D:mf3()" << endl; }
	void mf4 () { cout << "D:mf4()" << endl; }
};


int main () 
{
	Derived d;
	int x = 0;

	//d.mf1 ();//没问题，调用Derived::mf1
	////d.mf1 (x);//错误!因为Derived::mf1 遮掩了Base::mf1
	//d.mf2 () ;//没问题，调用Base::mf2
	//d.mf3 ();//没问题，调用Derived::mf3
	////d.mf3 (x);//错误!因为Derived::mf3 遮掩了Base::mf3

	d.mf1 ();//没问题，调用Derived::mf1
	d.mf1 (x);//现在没问题了调用Base::mf1
	d.mf2 () ;//没问题，调用Base::mf2
	d.mf3 ();//没问题，调用Derived::mf3
	d.mf3 (x);//现在没问题了，调用Base: :mf3
	d.Base::mf1();

	return 0;
}  

