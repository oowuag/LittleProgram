#include <stdio.h>
#include <math.h>

struct Beetle
{
    double x;
    double y;
    Beetle(double xx = 0, double yy = 0)
        : x(xx)
        , y(yy)
    {
    }
};

int main()
{
    const int N = 1000;
    const double stepN = 2;

    Beetle A[5];
    A[0] = Beetle(0, 0);
    A[1] = Beetle(N, 0);
    A[2] = Beetle(N, N);
    A[3] = Beetle(0, N);

    int nCnt = 0;
    while (true)
    {
        printf("%d,", nCnt);
        A[4] = A[0];
        for (int i = 0; i < 4; i++)
        {
            double lenX = A[i + 1].x - A[i].x;
            double lenY = A[i + 1].y - A[i].y;
            double lenXY = sqrt(lenX * lenX + lenY * lenY);
            A[i].x += (lenX / lenXY) * stepN;
            A[i].y += (lenY / lenXY) * stepN;
            printf("[%d],%f,%f,", i, A[i].x, A[i].y);
        }
        printf("\n");
        nCnt++;
        double lenErr = sqrt((A[0].x - (N / 2)) * (A[0].x - (N / 2)) + (A[0].y - (N / 2)) * (A[0].y - (N / 2)));
        if (lenErr < stepN) {
            break;
        }
    }

    return 0;
}