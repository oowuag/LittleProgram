#include <iostream>
using namespace std;

typedef int INT;

const int SIZE = 50;
int *q=(int*)(&SIZE);


struct sA 
{
	unsigned char version[SIZE];
};


class A
{
public:
	A()
		:ssa()
	{
	}
	sA ssa;
};

int main()
{
	unsigned char *pArray = new unsigned char[100];
	pArray[0] = 2;
	pArray[1] = 2;
	int num0 = pArray[1] * 0x100 + pArray[0];
	cout << num0 << endl; //0x202


	A a;
	cout << SIZE << endl;
	*q = 40; //runtime error
	cout << SIZE << endl;
	sA ssssa;
	cout << sizeof(ssssa) << endl; //runtime error


	return 0;
}