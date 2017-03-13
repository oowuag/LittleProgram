#include <iostream>
using namespace std;

typedef void VOID;
typedef char CHAR;

class A
{
    virtual VOID FooBar(CHAR *);
};
class B : public A
{
    virtual VOID FooBar(CHAR *);
};

int main()
{
	int i = 3;
	while ( i-- )
	{
		cout << "hello" << endl;
	}

	return 0;
}
