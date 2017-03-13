#include <iostream>
using namespace std;
class Base
{
public:
    Base()
    { 
        cout << "Base:Constructor"<<endl;
    }
    virtual ~Base()
    {
        cout << "Base:Destructor"<<endl;
    }
    virtual void virtualMethod()
    {
        cout << "Base:virtualMethod of foo\n";
    }

    void normalMethod()
    {
        cout << "Base:normalMethod of foo\n";
    }
};
class Derived: public Base
{
    //Doing a lot of jobs by extending the functionality
public:
    Derived()
    {
        cout << "Derived:Constructor"<<endl;
        virtualMethod();
    }
    virtual ~Derived()
    {
        cout << "Derived:Destructor"<<endl;
        virtualMethod();
    }

    void normalMethod()
    {
        cout << "Derived:normalMethod of foo\n";
    }
};
void foo()
{
    //Base & Var = Derived();
    Base*pVar = new Derived;
    delete pVar;
}
void main()
{
    foo();
}
