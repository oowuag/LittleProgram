
#include "SnsLogPrinter.h"

#include <iostream>
using namespace std;

//#pragma pack(4)
class A
{
public:
	char a;
	double b;
};
//#pragma pack()


int main()
{
	cout << sizeof(A) << endl;

	//SnsLogPrinter::Instance()->SetFileName("abc.txt");

	//FILE* fp = SnsLogPrinter::Instance()->GetLogFile();
	//fprintf(fp, "abcabc");


	//SnsLogPrinter::Instance()->SetFileName("def.txt");
	//fp = SnsLogPrinter::Instance()->GetLogFile();
	//fprintf(fp, "def");


	return 0;
}