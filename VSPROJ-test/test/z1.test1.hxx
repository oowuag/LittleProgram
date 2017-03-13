#include <stdio.h>
#include <string.h>

int test1()
{
    char ch[6] =  {"hel"};
    ch[4] = 'o';
    char temp[] = {"wor"};
    strcat(ch, temp);

    ch[0] = 'A';

    printf("%s", ch);

    return 0;
}

#define debug(fmt, ...) fprintf(stdout, "%s(%d): " fmt "\n" , __FUNCTION__ , __LINE__, ## __VA_ARGS__)

int main()
{
    debug("%d", 1);
    char* pc = new char[10];
    memset(NULL, 0, 10);
    if (pc == NULL) {
        return 0;
    }
    delete[] pc;

    return 0;
}