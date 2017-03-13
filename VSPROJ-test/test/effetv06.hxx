#include <iostream> 
using namespace std; 

void Test() //throw() 
{ 
	cout<<"抛出一个异常,测试!"<<endl; 
	throw 5; 
} 

void main() 
{ 
	try 
	{ 
		Test(); 
	} 
	catch(int value) 
	{ 
		cout<<"接收到异常value="<<value<<endl; 
	} 
} 