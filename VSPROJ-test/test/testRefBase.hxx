#include <iostream>
#include "RefBase/StrongPointer.h"
#include "RefBase/RefBase.h"
using namespace std;

template <typename T>
struct ncsp
{
    typedef android::sp<T> sp;
    typedef android::wp<T> wp;
};

class A : public android::RefBase
{
public:
    A(int x = 0) : m_a(x) {} 
    void show() {printf("%d", m_a);}
    int m_a;
};




int main()
{
    {
        android::sp<A> spInt = new A(3);
    }


    A* a1 = NULL;
    A* a2 = NULL;
    a1 = new A(31);
    a2 = new A(42);
    {
        ncsp<A>::sp ssp1(a1);
        const ncsp<A>::sp ssp2(a2);
        ssp1 = ssp2;
    }

    {
        ncsp<A>::sp ptr1(NULL);
        ncsp<A>::sp ptr2=ptr1;
        ptr1=ptr2;
        ptr2=ptr1;
    }

    {
        ncsp<A>::wp prt1(new A(333));
        ncsp<A>::sp ptr2 = prt1.promote();
    }

    a1 = new A(31);
    a2 = new A(42);
    {
        ncsp<A>::sp ptr1(NULL);
        ptr1 = a1;
        ptr1 = a2;
    }




    return 0;
}