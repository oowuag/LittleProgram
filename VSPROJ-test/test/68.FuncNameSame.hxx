#include <stdio.h>

class A
{
public:
    void testA()
    {
        printf("A:testA()\n");
    }
};

class B : public A
{
public:
    void testA()
    {
        printf("B:testA()\n");
        A::testA();
    }
};



int main()
{

    B b;
    b.testA();

}