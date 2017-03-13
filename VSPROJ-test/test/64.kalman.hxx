#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double frand()
{
	return 2*((rand()/(double)RAND_MAX) - 0.5);
}

int main() {

	//initial values for the kalman filter
	double x_est_last = 0;
	double P_last = 0;
	//the noise in the system
	//double Q = 0.022;
	//double R = 0.617;
	double Q = 0.00001;
	double R = 0.0028;

	double K;
	double P;
	double P_temp;
	double x_temp_est;
	double x_est;
	double z_measured; //the 'noisy' value we measured
	double z_real = 0.5; //the ideal value we wish to measure

	srand(0);

	//initialize with a measurement
	x_est_last = z_real + frand() * 0.09;	

	double sum_error_kalman = 0;
	double sum_error_measure = 0;
	int N = 100;
	for (int i=0;i<N;i++) {
		//do a prediction
		x_temp_est = x_est_last;
		P_temp = P_last + Q;
		//calculate the Kalman gain
		K = P_temp * (1.0/(P_temp + R));
		//measure
		z_measured = z_real + frand()*0.09; //the real measurement plus noise
		//correct
		x_est = x_temp_est + K * (z_measured - x_temp_est); 
		P = (1- K) * P_temp;
		//we have our new system

		printf("##%d   position: %6.3f, K:%f \n", i, z_real, K);
		printf(" Mesaured position: %6.3f [diff:%.3f]\n",z_measured,fabs(z_real-z_measured));
		printf(" Kalman   position: %6.3f [diff:%.3f]\n",x_est,fabs(z_real - x_est));

		sum_error_measure += fabs(z_real-z_measured) * fabs(z_real-z_measured);
		sum_error_kalman += fabs(z_real - x_est) * fabs(z_real - x_est);

		//update our last's
		P_last = P;
		x_est_last = x_est;
	}

	printf("Total error if using raw measured:  %f\n",sum_error_measure/N);
	printf("Total error if using kalman filter: %f\n",sum_error_kalman/N);
	printf("Reduction in error: %d%% \n",100-(int)((sum_error_kalman/sum_error_measure)*100));

	return 0;
}