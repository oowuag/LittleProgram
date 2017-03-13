#include<iostream>
#include<fstream>
#include <math.h>
using namespace std;

//1.0
//class Base {
//private:
//protected:
//       virtual bool ProcessMain(void)
//	   {
//		   cout << "Base::ProcessMain" << endl;
//		   return true;
//	   }
//public:
// void run(){ProcessMain();}
//
//};
//
//class Derived : public Base{
//protected:
// //using Base::ProcessMain;
// bool ProcessMain(void)
// {
//	 cout << "Derived::ProcessMain" << endl;
//	 return true;
// }
//};
//
//void main()
//{
//	Base *p = new Derived;
//	p->run();
//}

//2.0
//class base{
//public:
// void test(){
//  cout << "base::test()" << endl;
// }
// void test(int){
//  cout << "base::test(int)" << endl;
// }
//};
//
//class derived : public base{
//public:
//	using base::test;
// void test(){
//  cout << "derived::test()" << endl;
// }
//};
//
//void main()
//{
//	//base *p = new derived;
//	//p->test(2);
//	derived d;
//	d.test(3);
//}

//3.0
class base{
public:
 virtual void test(){
  cout << "base::test()" << endl;
 }
 virtual void test(double){
  cout << "base::test(double)" << endl;
 }
 void test(int){
  cout << "base::test(int)" << endl;
 }
};

class derived : public base{
public:
 void test(){
  cout << "derived::test()" << endl;
 }
 using base::test;
};

class A : public derived{
public:
 void test(double){
  cout << "A::test(double)" << endl;
 }
};

int main(int argc, char **argv){
 base *pb = new A;
 pb->test(2.4);
 return 0;
}