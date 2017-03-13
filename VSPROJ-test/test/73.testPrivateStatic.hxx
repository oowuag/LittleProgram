#include <stdio.h>
#include <Windows.h>

class A {
public:
    static bool FuncA()
    {
        printf("A::FuncA()");
        return true;
    }
};

class NHDiagSaveRecordFile;
#define DIAGRECORDID_Max 23

int main()
{
    //A::FuncA();

    //LONG lSize = 0;
    //INT x = (lSize - 1) / 16;
    //DWORD dwBufDataSize = 32 + (((lSize - 1) / 16 + 1) * 16);

    INT x = 0xFFFF0000;
    SHORT y = x;


    NHDiagSaveRecordFile*       m_apcSaveRecordFile[DIAGRECORDID_Max];
    printf("%d\n", sizeof(m_apcSaveRecordFile));

    return 0;
}