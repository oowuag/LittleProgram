#include <stdlib.h>
#include <math.h>                           /* math functions                */

//#define REAL float
#define REAL double


inline static REAL sqr(REAL x) {
	return x*x;
}

int linreg(int n, REAL x[], REAL y[], REAL* b, REAL* m, REAL* r);

//y=m*x+b
//n = number of data points
//x,y  = arrays of data
//*b = output intercept
//*m  = output slope
//*r = output correlation coefficient (can be NULL if you don't want it)

int linreg(int n, const REAL x[], const REAL y[], REAL* m, REAL* b, REAL* r)
{
	REAL   sumx = 0.0;                        /* sum of x                      */
	REAL   sumx2 = 0.0;                       /* sum of x**2                   */
	REAL   sumxy = 0.0;                       /* sum of x * y                  */
	REAL   sumy = 0.0;                        /* sum of y                      */
	REAL   sumy2 = 0.0;                       /* sum of y**2                   */

	for (int i=0;i<n;i++)   
	{ 
		sumx  += x[i];       
		sumx2 += sqr(x[i]);  
		sumxy += x[i] * y[i];
		sumy  += y[i];      
		sumy2 += sqr(y[i]); 
	} 

	REAL denom = (n * sumx2 - sqr(sumx));
	if (denom == 0) {
		// singular matrix. can't solve the problem.
		*m = 0;
		*b = 0;
		*r = 0;
		return 1;
	}

	*m = (n * sumxy  -  sumx * sumy) / denom;
	*b = (sumy * sumx2  -  sumx * sumxy) / denom;
	if (r!=NULL) {
		/* compute correlation coeff     */
		*r = (sumxy - sumx * sumy / n) / sqrt((sumx2 - sqr(sumx)/n) * (sumy2 - sqr(sumy)/n));
	}

	return 0; 
}

int main()
{
	return 0;
}