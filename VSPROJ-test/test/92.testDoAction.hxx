#include <iostream>
using namespace std;

class A
{
public:
    void f() { f(0); printf("A::f(0)");}
    virtual void f(int i) { printf("A::f_i(%d)", i); };
};

class B : public A
{
public:
    virtual void f(int i) {  printf("B::f_i(%d)", i); };
};

template <typename REQ>
class FFF
{
public:
    REQ* curReq;
    void foo() { curReq->f();}
};


int main()
{
    B* pb = new B();
    pb->f(1);

    FFF<B> fb;
    fb.foo();

    return 0;
}