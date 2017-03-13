#include <iostream>
#include <cmath>
using namespace std;

double F1(double x); //Ҫ���ĺ���
double F2(double x); //Ҫ���ĺ�����һ�׵�������  
double Newton(double x0, double e);//ͨ��Newton�����ӳ���
int main()
{
    double x0 = 3.14159/4;
    double e = 10E-6;
    
    printf("x = %f\n", Newton(x0, e));

    return 0;       
}
double F1(double x) //Ҫ���ĺ���   
{   
    return  cos(x) - x;   
}   
double F2(double x) //Ҫ���ĺ�����һ�׵�������   
{   
    return  -sin(x) - 1;   
}   
double Newton(double x0, double e)//ͨ��Newton�����ӳ���   
{   
    double  x1;   

    do
    {   
        x1 = x0;    
        x0 = x1 - F1(x1) / F2(x1);   

		cout << "x0:= " << x0 << endl;
    } while (fabs(x0 - x1) > e);   
          
    return x0;   //������x0��x1��ƽ��ֵ����� 
} 
