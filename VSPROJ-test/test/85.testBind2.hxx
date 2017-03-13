#include <iostream>
using namespace std;



class CTTA
{
public:
	int MyAdd(int a,int b){
		int c = a + b;
		printf("%d+%d=%d\n",a,b,c);
		return c;
	}
};
class CTTB
{
public:
	int MySub(int a,int b){
		int c = a - b;
		printf("%d-%d=%d\n",a,b,c);
		return c;
	}
};
class CTTC
{
public:
	int MyMax(int a,int b){
		int c = a>=b?a:b;
		printf("MyMax %d %d is %d\n",a,b,c);
		return c;
	}
};

typedef int (*pAddFunc)(int,int);
int Add (int a, int b) { return a+b; }

int test1()
{	
	CTTA cobj_a;
	CTTB cobj_b;
	CTTC cobj_c;

    pAddFunc add = &Add;
    cout<<(*add)(3,4)<<endl;

	//function func;

	//func = bind(cobj_a, CTTA, MyAdd);
	//func(4,3); //4+3

	//func = bind(cobj_b, CTTB, MySub);
	//func(4,3); //4-3

	//func = bind(cobj_c, CTTC, MyMax);
	//func(4,3); //max(4,3)

	return 0;
}


int main()
{
    test1();

    return 0;
}