// advance example
#include <iostream>     // std::cout
#include <iterator>     // std::advance
#include <list>         // std::list

class A
{
public:
    A(int a)
        :m_a(a)
    {
        std::cout << "A:A()" << m_a << std::endl;
    }

    ~A()
    {
        std::cout << "~A:A()" << m_a << std::endl;
    }
    int a()
    {
        return m_a;
    }
private:
    int m_a;
};

int main ()
{
    std::list<A*> mylist;
    for (int i=0; i<10; i++) {
        A* pA = new A(i);
        mylist.push_back(pA);
    }

    std::cout << "###########" << std::endl;
    for (std::list<A*>::iterator it = mylist.begin(); it != mylist.end();) {
        A* pA = *it;
        std::cout << pA->a() << std::endl;
        delete pA;
        it = mylist.erase(it);
    }
    mylist.clear();

    return 0;
}