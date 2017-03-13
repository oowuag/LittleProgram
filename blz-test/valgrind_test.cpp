#include <stdio.h>

int main(int argc, char* argv[])
{
    char* ptr = new char[20];

    delete ptr;
    ptr = 0;  // Comment me out and watch me crash and burn.
    delete ptr;
}