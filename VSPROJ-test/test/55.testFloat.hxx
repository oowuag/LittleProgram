#include <Windows.h>
#include <iostream>
using namespace std;


BYTE  
getDopTimes10(FLOAT fDop)
{
    BYTE	byDop = 0;

    if (fDop > 25.5F)  {
        /* 25.5 is Max */
        byDop = 255;
    }
    else if (fDop <= 0.0F)  {
        /* < 0.0 is Err */
        byDop = 0;
    }
    else if (fDop < 0.1F)   {
        /*  0.1 is Min for OD-2D */
        byDop = 1;
    }
    else  {
        byDop = static_cast< BYTE >(fDop * 10.0F);
    }
    return byDop;
}


int main()
{
	float x= 0.0F;

	for(BYTE i = 1; i > 0; i++)
	{
		x = i/10.0F;// + 0.000001F;
		BYTE ret = getDopTimes10(x);
		//if (ret != i) 
		{
			printf("%d, %f, %d\n", i, x, ret);
		}
	}


	return 0;
}