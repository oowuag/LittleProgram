//test: >cl Test.cpp /d1reportSingleClassLayout[className]
// /d1reportAllClassLayout

#include <iostream>
using namespace std;

struct S
{
	char x;
	int y;
	double z;
};
class TestClass
{
private:
	char y;
	double z;
	int x;
};
//base
class Base
{
private:
	int x;
public:
	virtual void f1();
	virtual int g1();
};
//Derived
class Derived: public Base
{
private:
	char y;
public:
	virtual float f2();
};
//Derived2
class Derived2: public Base
{
private:
	double z;
public:
	virtual void f1();
	virtual float v2();
	int f3();
};
//
class Base2
{
private:
	int yy;
public:
	virtual void f1();
	virtual void g2();
};
//∂‡÷ÿºÃ≥–
class Derived3: public Base, public Base2
{
private:
	double zz;
public:
	virtual void f1();
	virtual void g3();
};

class Derived4 : public Derived3
{
};
//

class A
{
public:
	//void funcA(){};
private:
	int a;
	int aa;
	int aaa;
};

class B : virtual public A
{
public:
	//void funcA(){};
private:
	int b;
};

class C : virtual public A
{
public:
	//void funcA(){};
private:
	int c;
};

class D : public B, public C
{
public:
	//void funcA(){};
private:
	int d;
};

int main()
{
	cout << sizeof(A) << endl;
	cout << sizeof(B) << endl;
	cout << sizeof(C) << endl;
	cout << sizeof(D) << endl;


	return 0;
}