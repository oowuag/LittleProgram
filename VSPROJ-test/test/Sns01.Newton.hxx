#include <iostream>
#include <cmath>
using namespace std;

double F1(double x); //要求解的函数
double F2(double x); //要求解的函数的一阶导数函数  
double Newton(double x0, double e);//通用Newton迭代子程序
int main()
{
    double x0 = 3.14159/4;
    double e = 10E-6;
    
    printf("x = %f\n", Newton(x0, e));

    return 0;       
}
double F1(double x) //要求解的函数   
{   
    return  cos(x) - x;   
}   
double F2(double x) //要求解的函数的一阶导数函数   
{   
    return  -sin(x) - 1;   
}   
double Newton(double x0, double e)//通用Newton迭代子程序   
{   
    double  x1;   

    do
    {   
        x1 = x0;    
        x0 = x1 - F1(x1) / F2(x1);   

		cout << "x0:= " << x0 << endl;
    } while (fabs(x0 - x1) > e);   
          
    return x0;   //若返回x0和x1的平均值则更佳 
} 
