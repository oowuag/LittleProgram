#include <iostream>  
#include <exception>  
using namespace std;  
void myunexpected () {  
	cerr << "unexpected called/n";  
	throw 0;     // throws int (in exception-specification)  
}  
void myfunction () throw (int) {  
	throw 'x';   // throws char (not in exception-specification)  
}  
int main (void) {  
	set_unexpected (myunexpected);  
	try {  
		myfunction();  
	}  
	catch (int) {  
		cerr << "caught int/n";  
	}  
	catch (...) {  
		cerr << "caught other exception (non-compliant compiler?)\n";  
	}  
	return 0;  

	//有问题！！！查看gcc运行情况
}  

