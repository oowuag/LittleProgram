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
	using Base::mf1; //��Base class ����Ϊmf1 ��mf3�����ж���
	using Base::mf3; //��Derived �������ڶ��ɼ�(����public)

	virtual void mf1() { cout << "D:mf1()" << endl; }
	void mf3 () { cout << "D:mf3()" << endl; }
	void mf4 () { cout << "D:mf4()" << endl; }
};


int main () 
{
	Derived d;
	int x = 0;

	//d.mf1 ();//û���⣬����Derived::mf1
	////d.mf1 (x);//����!��ΪDerived::mf1 ������Base::mf1
	//d.mf2 () ;//û���⣬����Base::mf2
	//d.mf3 ();//û���⣬����Derived::mf3
	////d.mf3 (x);//����!��ΪDerived::mf3 ������Base::mf3

	d.mf1 ();//û���⣬����Derived::mf1
	d.mf1 (x);//����û�����˵���Base::mf1
	d.mf2 () ;//û���⣬����Base::mf2
	d.mf3 ();//û���⣬����Derived::mf3
	d.mf3 (x);//����û�����ˣ�����Base: :mf3
	d.Base::mf1();

	return 0;
}  

