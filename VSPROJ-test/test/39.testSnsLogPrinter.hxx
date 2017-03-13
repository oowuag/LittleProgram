#include "SnsLogPrinter.h"
#include <iostream>
using namespace std;

double foo()
{

	while(1)
	{
		cout << "foo" <<endl;
		break;
	}
}

int main()
{
	//SnsLogPrinter::Instance()->SetFileName("abc.txt");


	//for(int i=0; i< 3335; i++)
	//{
	//	SnsLogPrinter::Instance()->WriteLog("$GPGSA,A,1,29,05,07,,,,,,,,,,74.06,74.06,1.00*08\n");
	//	SnsLogPrinter::Instance()->WriteLog("$GPRMC,142246.80,V,0000.0000,N,00000.0000,E,0.0,0.0,130813,0.0,,N*6f\n");

	//}


	//SnsLogPrinter::Instance()->Stop();



	foo();



	return 0;
}
