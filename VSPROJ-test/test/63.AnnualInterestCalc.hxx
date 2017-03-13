#include <iostream>
using namespace std;

int main()
{
	double dMonth = 5524.96;

	double dTotalRate = 0.0f;

	for(int i=0; i<60; i++)
	{
		double rate = (300000 - dMonth* i) * 0.04/12; 
		dTotalRate += rate;
	}

	return 0;
}