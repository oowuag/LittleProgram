//p.39.

#include <iostream>
using namespace std;

class Person {
private:
	virtual void f()
    {
        cout << "Person::f()" << endl;
    }
};
class Student: private Person { 
private:
	//virtual void f()
 //   {
 //       cout << "Student::f()" << endl;
 //   }
	void f2(){ f(); }
};
void eat(const Person& p) {}
void study(const Student& s) {}

int main()
{
	Person p;
	Student s;
	eat (p) ;
//	eat (s) ; //如果是public继承，就是正确的
}
