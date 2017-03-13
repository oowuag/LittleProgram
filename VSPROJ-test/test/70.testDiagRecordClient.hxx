#include <stdio.h>                                                                                                                           
#include <string.h> 
#include <Windows.h>
#include <list>

#define uint32_t long
#define NC_BOOL BOOL
#define NC_FALSE FALSE
#define NC_TRUE TRUE

// MMDiag Frame
struct NHMMDiagFrame
{
    BYTE            byData[16];
    NHMMDiagFrame()
    {
        memset(byData, 0, sizeof(byData));
    }
    ~NHMMDiagFrame()
    {
    }
};

// MMDiag Record
class NHMMDiagRecord
{
public:
    explicit NHMMDiagRecord(DWORD wCnt)
        :m_wCnt(wCnt)
        ,m_psFrame(NULL)
        ,m_bIsValid(NC_FALSE)
    {
        m_psFrame = new NHMMDiagFrame[m_wCnt];
    }

    ~NHMMDiagRecord()
    {
        if (NULL != m_psFrame) {
            delete[] m_psFrame;
            m_psFrame = NULL;
        }
    }

    NHMMDiagFrame& getFrame(DWORD nCnt)
    {
        if (NULL != m_psFrame) {
            return m_psFrame[nCnt];
        }
    }

    DWORD getFrameCnt()
    {
        return m_wCnt;
    }

    VOID fillGpsTimeTo1stFrame(BYTE byGpsTime[])
    {
        if (NULL != m_psFrame) {
            memcpy(&(m_psFrame[0].byData[0]), byGpsTime, 3);
            m_bIsValid = NC_TRUE;
        }
    }

    NC_BOOL isValid()
    {
        return m_bIsValid;
    }

    /**
    * @brief Copy constructor
    */
    NHMMDiagRecord(const NHMMDiagRecord& ref)
    {
        m_wCnt = ref.m_wCnt;
        m_psFrame = new NHMMDiagFrame[m_wCnt];
        for (DWORD wN=0; wN<=(m_wCnt-1); ++wN) {
            memcpy((char*)m_psFrame[wN].byData, (char*)ref.m_psFrame[wN].byData, 16);
        }
        m_bIsValid = ref.m_bIsValid;
    }


private:
    DWORD            m_wCnt;
    NHMMDiagFrame   *m_psFrame;
    NC_BOOL         m_bIsValid;

    /**
    * @brief Copy constructor(Forbidden).
    */
    NHMMDiagRecord& operator = (const NHMMDiagRecord& ref);

};

class NHDiagRecordBuff
{
public:

    // constructor
    explicit NHDiagRecordBuff();

    // destructor
    ~NHDiagRecordBuff();

    // trans input data to record
    VOID inPut(const void* buff, const uint32_t size);

    // trans record to RecordFile buffer
    VOID outPut(std::list<NHMMDiagRecord>& outListRecord);

    // set Gps Time
    VOID setGpsTimeToAllRecord(BYTE byGpsTime[]);

private:
    std::list<NHMMDiagRecord>   m_listRecord;
    NC_BOOL                     m_bWriteFileFlag;

    /**
    * @brief Copy constructor(Forbidden).
    */
    NHDiagRecordBuff(const NHDiagRecordBuff& ref);

    /**
    * @brief Copy constructor(Forbidden).
    */
    NHDiagRecordBuff& operator = (const NHDiagRecordBuff& ref);
};

// NHDiagRecordBuff
// constructor
NHDiagRecordBuff::NHDiagRecordBuff()
:m_bWriteFileFlag(NC_FALSE)
{
}

// destructor
NHDiagRecordBuff::~NHDiagRecordBuff()
{
}

// trans input data to record
VOID NHDiagRecordBuff::inPut(const void* buff, const uint32_t size)
{
    if (size <= 0) {
        return;
    }

    DWORD wFrameCnt = 0;
    DWORD wLastFrameSize = 0;
    if (size <= 13) { //size <=13
        wFrameCnt = 1;
        wLastFrameSize = size;
    }
    else { // size > 13
        // calc FrameCnt
        wFrameCnt = (DWORD)((((size-13) -1) / 12) + 1) + 1;
        // calc last frame size
        wLastFrameSize = (DWORD)((size-13) % 12);
        if (0 == wLastFrameSize) {
            wLastFrameSize = 12;
        }
    }

    NHMMDiagRecord cRecord(wFrameCnt);

    const BYTE RECORD_2ND_PREFIX[4] = {0x00, 0x00, 0x00, 0xFF};
    char* pStrBuff = (char *)(buff);
    if (1 == wFrameCnt) {
        // pad 1s GpsTime later
        memcpy(&(cRecord.getFrame(0).byData[3]), &pStrBuff[0], wLastFrameSize);
    }
    else {
        DWORD wBuffPt = 0;
        // pad 1s GpsTime later
        memcpy(&(cRecord.getFrame(0).byData[3]), &pStrBuff[wBuffPt], 13);
        wBuffPt += 13;
        for (DWORD wN=1; wN<(wFrameCnt-1); ++wN) {
            // pad first 4 [00,00,00,FF]
            memcpy(&(cRecord.getFrame(wN).byData[0]), RECORD_2ND_PREFIX, 4);
            // pad last 12
            memcpy(&(cRecord.getFrame(wN).byData[4]), &pStrBuff[wBuffPt], 12);
            wBuffPt += 12;
        }
        // pad first 4 [00,00,00,FF]
        memcpy(&(cRecord.getFrame(wFrameCnt-1).byData[0]), RECORD_2ND_PREFIX, 4);
         // pad last 12
        memcpy(&(cRecord.getFrame(wFrameCnt-1).byData[4]), &pStrBuff[wBuffPt], wLastFrameSize);
    }

    if (m_listRecord.size() == 100) 
    {
        m_listRecord.pop_front();        
    }
    m_listRecord.push_back(cRecord);
}

// trans record to RecordFile buffer
VOID NHDiagRecordBuff::outPut(std::list<NHMMDiagRecord>& outListRecord)
{
    while (!m_listRecord.empty())
    {
        if (m_listRecord.front().isValid())
        {
            outListRecord.push_back(m_listRecord.front());
            m_listRecord.pop_front();
        }
    }
}

// set Gps Time
VOID NHDiagRecordBuff::setGpsTimeToAllRecord(BYTE byGpsTime[])
{
  for (std::list<NHMMDiagRecord>::iterator it= m_listRecord.begin(); it != m_listRecord.end(); ++it)
  {
      (*it).fillGpsTimeTo1stFrame(byGpsTime);
  }
}


int main()
{
    NHDiagRecordBuff cRecordBuff;
    //char strBuff[] = "0123456789";
    //char strBuff[] = "01234567890123456789";
    char strBuff3[] = "012345678901234567890123456789";
    char strBuff4[] = "0123456789012345678901234567890123456789";
    //char strBuff5[] = "0123456789012345678901234567890123456789012345678";
    char strBuff5[] = "0123456789012345678901234";


    //cRecordBuff.inPut(strBuff3, strlen(strBuff3));
    cRecordBuff.inPut(strBuff4, strlen(strBuff4));
    cRecordBuff.inPut(strBuff5, strlen(strBuff5));
    BYTE byGpsTime[3] = {0x11,0x22,0x33};
    cRecordBuff.setGpsTimeToAllRecord(byGpsTime);


    std::list<NHMMDiagRecord> outListRecord;

    cRecordBuff.outPut(outListRecord);



    DWORD nAllDataNum = 0;
    for (std::list<NHMMDiagRecord>::iterator it= outListRecord.begin(); it != outListRecord.end(); ++it)
    {
        nAllDataNum += (*it).getFrameCnt();
    }
    nAllDataNum *= 16;
    BYTE *pWriteData = new BYTE[nAllDataNum];
    memset(pWriteData, 0, nAllDataNum);
    DWORD nPtr = 0;
    while (!outListRecord.empty())
    {
        DWORD nCnt = outListRecord.front().getFrameCnt();
        for (INT i=0; i<nCnt; ++i)
        {
            memcpy(&pWriteData[nPtr*16], outListRecord.front().getFrame(i).byData, 16);
            nPtr++;
        }
        outListRecord.pop_front();
    }


    WORD wPtr = 0;
    wPtr = (wPtr - 1 + 1000) % 1000;


    return 0;
}
