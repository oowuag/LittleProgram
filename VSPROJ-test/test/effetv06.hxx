#include <iostream> 
using namespace std; 

void Test() //throw() 
{ 
	cout<<"�׳�һ���쳣,����!"<<endl; 
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
		cout<<"���յ��쳣value="<<value<<endl; 
	} 
} 