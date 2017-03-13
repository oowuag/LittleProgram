#include <stdio.h>

class AA
{
public:
	virtual void funcA(){
		printf("AA::funcA()!\n");
	}
};

class BB : public AA
{
private:
	void funcA(){
		printf("BB::funcA()!\n");
	}
};

int main()
{
	AA* pA = new BB();
	pA->funcA();
	delete pA;

	return 0;
}