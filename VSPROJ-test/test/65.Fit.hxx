// LeastSquare FitLine

#include<stdio.h>


#define N 10             //N - data number

float X[10] = {1.9,0.8,1.1,0.1,-0.1,4.4,4.6,1.6,5.5,3.4};
float Y[10] = {0.7,-1.0,-0.2,-1.2,-0.1,3.4,0.0,0.8,3.7,2.0};


double Sum_Average(float d[N])
{
	unsigned int i=0;
	float z=0;
	for(i=0;i<N;i++)
	{
		z = z + d[i];
	}
	z = z/N;
	return z;
}
double X_Y_By(float m[N],float n[N])
{
	unsigned int i=0;
	float z=0;
	for(i=0;i<N;i++)
	{
		z = z + m[i]*n[i];
	}
	return z;
}

double Squre_sum(float c[N])
{
	unsigned int i=0;
	float z=0;
	for(i=0;i<N;i++)
	{
		z = z + c[i]*c[i];
	}
	return z;
}

void Line_Fit(void)
{
	float K=0;               //fitline slope 
	float R=0;               //fitline intercept
	double x_sum_average=0;   //array X[N] all data average
	double y_sum_average=0;   //array Y[N] all data average
	double x_square_sum=0;    //array X[N] all data square sum
	double x_multiply_y=0;    //array X[N]*Y[N]
	x_sum_average= Sum_Average(X);
	y_sum_average= Sum_Average(Y);
	x_square_sum = Squre_sum(X);
	x_multiply_y = X_Y_By(X,Y);
	K = ( x_multiply_y - N * x_sum_average * y_sum_average)/( x_square_sum - N * x_sum_average*x_sum_average );
	R = y_sum_average - K * x_sum_average;
	printf("K = %f\n",K);
	printf("R = %f\n",R);
}


void main(void)
{
	Line_Fit();
}


