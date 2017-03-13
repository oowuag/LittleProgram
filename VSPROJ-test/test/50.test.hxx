#include <iostream>
using namespace std;

#ifdef MYDEBUG

#define private public

#endif


class B {
private:
	void out()
	{
		cout << "B::out()" << endl;
	}
};
