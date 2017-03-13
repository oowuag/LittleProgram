#include <iostream>
using namespace std;

int main()
{
	for (int i=0; i<25; i++)
	{
		int val = i*10;
		cout << val << "\t" ;//<< endl;
		cout << val/3.6 << "\t\t";
		cout << val/4.0 << "\t";
		cout << val*0.3 << endl;
	}

	return 0;
}