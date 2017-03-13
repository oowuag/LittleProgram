#include <stdio.h>
#include <Windows.h>
#include <list>

class classA {  
public:  
    template <typename T>
    void print(T& t) const {  
        printf("print:%d\n", t);  
    }  
}; 

int main()
{
    BYTE buffData[16] = {0x01,0x02,0x03,0x04,0x05};

    BYTE zbyNewTickTime[3] = { 0 };
    memcpy(zbyNewTickTime, buffData, 3);


    CHAR szSQL[16] = { 0 };
    _snprintf(szSQL, 16, "aa");
    _snprintf(szSQL, 16, "abcdef");
    _snprintf(szSQL, 16, "bbb");

    classA a;
    int x = 3.5;
    a.print(x);

    std::list<int> mlist;
    mlist.clear();
    mlist.clear();

        DWORD m_dwHeartBeatCnt = 0;      
        const DWORD MMDIAG_CONTROL_HEART_FLUSH_CNT = 12;  // 60*5s=300s

        m_dwHeartBeatCnt++;
        // check flush
        BOOL bFlush = FALSE;
        if (TRUE) {
            if (0 == m_dwHeartBeatCnt % MMDIAG_CONTROL_HEART_FLUSH_CNT) {
                // every 300s flush once
                bFlush = TRUE;
            }
            else if (1 == m_dwHeartBeatCnt % MMDIAG_CONTROL_HEART_FLUSH_CNT) {
                bFlush = FALSE;

            }
            else {
                // do nothing
            }
        }


    return 0;
}