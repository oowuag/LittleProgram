#include<iostream> 
using namespace std;

double fuc(double x, double y)  throw()
{
	if(y==0)
	{
		throw x;
    }
	return x/y;
}
void main()
{
	double res;
	try
    {
		res=fuc(2,3);
		cout<<"The result of x/y is : "<<res<<endl;
		res=fuc(4,0);
        cout<<"The result of x/y is : "<<res<<endl;
    }
	catch(double z)
    {
		cerr<<"error of dividing zero " << z << endl;
		exit(1);
	}
}