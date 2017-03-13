#include <iostream>
using namespace std;

int main()
{
	float x = 3.14159F;
	x = 0.000163F;
	char str[4] = {0};

	char *px = (char *)&x;
	for (int i=0; i < 4; i++) 
	{
		str[i] = *px++;
	}

	x = 0.00000000163F;
	x = x * 20.3F / 10.2;
	x += 0.00000000001F;


	return 0;
}