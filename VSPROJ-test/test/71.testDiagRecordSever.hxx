#include <list>
#include <Windows.h>


typedef enum DIAGRECORDID
{
    DIAGRECORDID_POWER,             ///< Power supply trace
    DIAGRECORDID_MODE,               ///< Mode management vehicle signal trace
    DIAGRECORDID_AVCLAN,            ///< AVC_LAN communication trace
    DIAGRECORDID_MOST,               ///< MOST communication trace
    DIAGRECORDID_KEY,                   ///< Key trace
    DIAGRECORDID_ERROR,             ///< Error trace
    DIAGRECORDID_GLOBALCAN,    ///< Global CAN communication trace
    DIAGRECORDID_CARPOSITION, ///< Car position out trace
    DIAGRECORDID_PFWATCHLOG,  ///< Application delivery platform function watch log
    DIAGRECORDID_PFUSELOG,        ///< Application delivery platform function use log
    DIAGRECORDID_BTCONNECT,    ///< Bluetooth communication trace
    DIAGRECORDID_BTINNER,          ///< Bluetooth trace
    DIAGRECORDID_WIFICONNECT, ///< WiFi communication trace
    DIAGRECORDID_WIFIINNER,       ///< WiFi trace
    DIAGRECORDID_CENTERCOM,    ///< Center communication trace
    DIAGRECORDID_VERSION,           ///< Software version information trace
    DIAGRECORDID_DCMCOM,          ///< DCM communication trace
    DIAGRECORDID_DSRCETC,           ///< DSRC ETC communication trace
    DIAGRECORDID_LOCALCAN,        ///< Local CAN communication trace
    DIAGRECORDID_RADIO,               ///< Radio wave reception trace
    DIAGRECORDID_DCUMEUCOM,   ///< DCU MEU communication trace
    DIAGRECORDID_USBIF,                 ///< USB I/F connection application inner trace

    DIAGRECORDID_Max
} ni_hallib_diagrecord_id;


#define MMDIAG_ERROR printf
#define MMDIAG_DG printf

// MMDiag Record
class NCDiagMMRecord
{
public:
    // constructor
    explicit NCDiagMMRecord(DWORD dwSize)
        : m_pData(NULL)
        , m_dwSize(dwSize)
    {
        m_pData = new BYTE[dwSize];
        memset(m_byTickTime, 0, 3);
        memset(m_pData, 0, dwSize);
    }
    // destructor
    ~NCDiagMMRecord()
    {
        if (NULL != m_pData) {
            delete[] m_pData;
            m_pData = NULL;
        }
    }

    // get data
    BYTE* getData()
    {
        return m_pData;
    }

    // get data size
    DWORD getDataSize()
    {
        return m_dwSize;
    }

    // get tick time
    BYTE* getTickTime()
    {
        return &m_byTickTime[0];
    }

    // trim special data size
    VOID trimDataSize(DWORD dwTrimSize)
    {
        if ((dwTrimSize > 0) && (dwTrimSize < m_dwSize)) {
            if (NULL == m_pData) {
                return;
            }
            // copy data
            BYTE *pNewData = new BYTE[dwTrimSize];
            memcpy(pNewData, m_pData, dwTrimSize);
            delete[] m_pData;
            // set new pointer and size
            m_pData = pNewData;
            m_dwSize = dwTrimSize;
        }
    }

    // pad tick time
    void padTickTime(BYTE byTickTime[])
    {
        memcpy(m_byTickTime, byTickTime, 3);
    }

    // pad Data
    void padData(DWORD dwNum, BYTE* pData, DWORD dwSize)
    {
        if (dwNum < m_dwSize) {
            memcpy(&(m_pData[dwNum]), pData, dwSize);
        }
    }

    // copy constructor
    NCDiagMMRecord(const NCDiagMMRecord& ref)
    {
        m_dwSize = ref.m_dwSize;
        m_pData = new BYTE[m_dwSize];
        memcpy((char*)m_pData, (char*)ref.m_pData, m_dwSize);
        memcpy((char*)m_byTickTime, (char*)ref.m_byTickTime, 3);
    }

private:
    BYTE    m_byTickTime[3];
    DWORD   m_dwSize;
    BYTE*   m_pData;

    friend bool compareRecordByTick(const NCDiagMMRecord& first, const NCDiagMMRecord& second);

    // assign constructor(Forbidden)
    NCDiagMMRecord& operator = (const NCDiagMMRecord& ref);
};

/// NCDIAG_MMCollectFile
// comparison, by tickcnt oder
bool compareRecordByTick(const NCDiagMMRecord& first, const NCDiagMMRecord& second)
{
    // compare from high to low bytes
    for (DWORD wN = 0; wN < 3; ++wN) {
        // smaller
        if (first.m_byTickTime[wN] < second.m_byTickTime[wN]) {
            return true;
        }
        // larger
        else if (first.m_byTickTime[wN] > second.m_byTickTime[wN]) {
            return false;
        }
        else {
        }
    }
    // equal
    return false;
}


class NCDIAG_MMCollectFile
{
public:

    bool collectFile();

private:
    // collect record from 1 file
    bool collectFileRecord(DIAGRECORDID eRecdID, std::list<NCDiagMMRecord>& listRecord, DWORD& dwRecSize, char* pFileName, DWORD dwFileSize);

    // push record to list
    DWORD pushRecordList(DIAGRECORDID eRecdID, std::list<NCDiagMMRecord>& listRecord, DWORD dwFrHead, DWORD dwFrEnd, BYTE* pData);

    // push normal record
    DWORD pushListNormalRecord(std::list<NCDiagMMRecord>& listRecord, DWORD dwFrHead, DWORD dwFrEnd, BYTE* pData);

    // push special record
    DWORD pushListSpecialRecord(std::list<NCDiagMMRecord>& listRecord, DWORD dwFrHead, DWORD dwFrEnd, BYTE* pData);


};

// collect record from 1 file
bool NCDIAG_MMCollectFile::collectFileRecord(DIAGRECORDID eRecdID, std::list<NCDiagMMRecord>& listRecord, DWORD& dwRecSize, char* pFileName, DWORD dwFileSize)
{
    // MMDIAG_DG("NCDIAG_MMCollectFile::collectFileRecord");

#if 0
    NCString szFileName(pFileName);
    NCFile cFile(szFileName);

    // check file size is OK
    DWORD dwFrameCnt = dwFileSize / 16;
    if ((dwFrameCnt < 1) || (dwFileSize % 16 != 0)) {
        MMDIAG_ERROR("collectFileRecord dwFrameCnt error");
        // remove file
        if (!cFile.remove()) {
            MMDIAG_ERROR("collectFileRecord Error deleting file");
        }
        return false;
    }

    // open file
    if (NC_FALSE == cFile.open(NC_FOM_ReadOnly)) {
        MMDIAG_ERROR("collectFileRecord fopen error");
        return NC_FALSE;
    }


    // read file data
    BYTE* pFileData = new BYTE[dwFileSize];
    DWORD result = cFile.read(pFileData, dwFileSize);
    if (result != dwFileSize) {
        MMDIAG_ERROR("collectFileRecord fread error");
        delete[] pFileData;
        cFile.close();
        return false;
    }
    cFile.close();

    // remove file
    if (!cFile.remove()) {
        MMDIAG_ERROR("collectFileRecord Error deleting file");
    }
    else {
        // MMDIAG_DG("collectFileRecord delete file sucess");
    }
#else
    // check file size is OK
    DWORD dwFrameCnt = dwFileSize / 16;
    if ((dwFrameCnt < 1) || (dwFileSize % 16 != 0)) {
        printf("collectFileRecord dwFrameCnt error");
        return false;
    }

    // open file
    FILE* pFile = fopen(pFileName, "rb");
    if (NULL == pFile) {
        printf("collectFileRecord fopen error");
        return false;
    }

    // read file data
    BYTE* pFileData = new BYTE[dwFileSize];
    memset(pFileData, 0, dwFileSize);
    DWORD result = fread(pFileData, 1, dwFileSize, pFile);
    if (ferror (pFile))
        printf ("Error read to myfile.txt\n");
    if (result != dwFileSize) {
        printf("collectFileRecord fread error");
        delete[] pFileData;
        fclose(pFile);
        return false;
    }
    fclose(pFile);

    //// remove file
    //if( remove(pFileName) != 0 ) {
    //    perror("Error deleting file");
    //}
    //else {
    //    puts("File successfully deleted");
    //}
#endif

    // pad record
    DWORD dwFrameHead = 0;
    DWORD dwFrameEnd = 0;
    DWORD dwDataP = 0;
    DWORD dw1RecordSize = 0;

    for (DWORD dwFrameN = 0; dwFrameN < dwFrameCnt; ++dwFrameN) {
        dwDataP = dwFrameN * 16;
        // check first 4 bytes [0x00 0x00 0x00 0xFF]
        if (0x00 == pFileData[dwDataP] && 0x00 == pFileData[dwDataP + 1]
            && 0x00 == pFileData[dwDataP + 2] && 0xFF == pFileData[dwDataP + 3]) {
            ++dwFrameEnd;
        }
        else {
            // 1st frame default have TickTime
            if (dwFrameN != 0) {
                // add record to list
                dw1RecordSize = pushRecordList(eRecdID, listRecord, dwFrameHead, dwFrameEnd, pFileData);
                dwRecSize += dw1RecordSize;
                // next head
                dwFrameHead = dwFrameEnd + 1;
                dwFrameEnd = dwFrameHead;
            }
        }
    }
    // add last record
    if ((dwFrameHead <= dwFrameEnd) && (dwFrameEnd <= (dwFrameCnt-1))) {
        dw1RecordSize = pushRecordList(eRecdID, listRecord, dwFrameHead, dwFrameEnd, pFileData);
        dwRecSize += dw1RecordSize;
    }

    delete[] pFileData;

    return true;
}

// push record to list
DWORD NCDIAG_MMCollectFile::pushRecordList(DIAGRECORDID eRecdID, std::list<NCDiagMMRecord>& listRecord, DWORD dwFrHead, DWORD dwFrEnd, BYTE* pData)
{
    // special data size process
    // BTCONNECT  WIFICONNECT
    if ((DIAGRECORDID_BTCONNECT == eRecdID) || (DIAGRECORDID_WIFICONNECT == eRecdID)) {
        return pushListSpecialRecord(listRecord, dwFrHead, dwFrEnd, pData);
    }
    else {
        return pushListNormalRecord(listRecord, dwFrHead, dwFrEnd, pData);
    }
}

// push normal record to list
DWORD NCDIAG_MMCollectFile::pushListNormalRecord(std::list<NCDiagMMRecord>& listRecord, DWORD dwFrHead, DWORD dwFrEnd, BYTE* pData)
{
    // MMDIAG_DG("NCDIAG_MMCollectFile::pushListNormalRecord");

    // check head first 4 bytes [0x00 0x00 0x00 0xFF]
    if ((0x00 == pData[dwFrHead * 16]) && (0x00 == pData[dwFrHead * 16 + 1])
        && (0x00 == pData[dwFrHead * 16 + 2]) && (0xFF == pData[dwFrHead * 16 + 3])) {
        return 0;
    }

    // check head/end
    if (dwFrHead > dwFrEnd) {
        return 0;
    }

    // head start data point
    DWORD dwDataRecordP = dwFrHead * 16;
    // calc frame count
    DWORD dwRecordFrameCnt = dwFrEnd - dwFrHead + 1;
    // calc real pad data size
    DWORD dwDataSize = dwRecordFrameCnt * 16;


    NCDiagMMRecord cMMRecord(dwDataSize);

    // pad gps time, use frame firt 3 bytes
    cMMRecord.padTickTime(&pData[dwDataRecordP]);

    DWORD dwRecordP = 0;
    for (DWORD dwRecordFrameN = 0; dwRecordFrameN <= (dwRecordFrameCnt-1); ++dwRecordFrameN) {
        // pad all data
        cMMRecord.padData(dwRecordP, &pData[dwDataRecordP], 16);
        dwRecordP += 16;
        dwDataRecordP += 16; // frame++, then data pointer+16
    }

    listRecord.push_back(cMMRecord);

    return dwDataSize;
}

// push special record to list
DWORD NCDIAG_MMCollectFile::pushListSpecialRecord(std::list<NCDiagMMRecord>& listRecord, DWORD dwFrHead, DWORD dwFrEnd, BYTE* pData)
{
    // MMDIAG_DG("NCDIAG_MMCollectFile::pushListSpecialRecord");

    // check head first 4 bytes [0x00 0x00 0x00 0xFF]
    if ((0x00 == pData[dwFrHead * 16]) && (0x00 == pData[dwFrHead * 16 + 1])
        && (0x00 == pData[dwFrHead * 16 + 2]) && (0xFF == pData[dwFrHead * 16 + 3])) {
        return 0;
    }

    // check head/end
    if (dwFrHead > dwFrEnd) {
        return 0;
    }

    // head start data point
    DWORD dwDataRecordP = dwFrHead * 16;
    // calc frame count
    DWORD dwRecordFrameCnt = dwFrEnd - dwFrHead + 1;
    // calc real pad data size
    DWORD dwDataSize = 13 + (dwRecordFrameCnt-1) * 12;

    if (dwDataSize >= 0xFFFF) {
        MMDIAG_ERROR("NCDIAG_MMCollectFile::pushListSpecialRecord  ERROR dw1RecordDataSize");
        return 0;
    }

    NCDiagMMRecord cMMRecord(dwDataSize);

    // pad gps time, use frame firt 3 bytes
    cMMRecord.padTickTime(&pData[dwDataRecordP]);
    dwDataRecordP += 3;
    // get real size
    WORD wRealSize = 0;
    memcpy(&wRealSize, &pData[dwDataRecordP], 2);
    dwDataRecordP += 2;
    if ((wRealSize > (dwDataSize - 2)) || wRealSize <= 0) {
        MMDIAG_ERROR("NCDIAG_MMCollectFile::pushListSpecialRecord  ERROR wRealSize");
        return 0;
    }

    // pad 1st data
    DWORD dwRecordP = 0;
    // frame 0
    cMMRecord.padData(dwRecordP, &pData[dwDataRecordP], 11); // 16-3-2=11
    dwRecordP += 11;
    dwDataRecordP += 11; // frame 0, data pointer+16

    for (DWORD dwRecordFrameN = 1; dwRecordFrameN <= (dwRecordFrameCnt-1); ++dwRecordFrameN) {
        // pad real data
        cMMRecord.padData(dwRecordP, &pData[dwDataRecordP + 4], 12);
        dwRecordP += 12;
        dwDataRecordP += 16; // frame++, then data pointer+16
    }

    // trim data size
    cMMRecord.trimDataSize(wRealSize);

    // MMDIAG_DG("NCDIAG_MMCollectFile::pushListSpecialRecord special size:%d, data:%.*s",
    //    cMMRecord.getDataSize(), cMMRecord.getDataSize(), cMMRecord.getData());


    listRecord.push_back(cMMRecord);

    return wRealSize;
}


bool NCDIAG_MMCollectFile::collectFile()
{
    //char fileName[] = "C:\\SnsProj\\17cy\\power_593_44_111203_DONE";
    //DIAGRECORDID eRecdID = DIAGRECORDID_POWER;

    char fileName[] = "C:\\SnsProj\\17cy\\btconnect_585_44_512410_DONE";
    DIAGRECORDID eRecdID = DIAGRECORDID_BTCONNECT;

    FILE * pFile = fopen (fileName , "rb" );
    if (pFile==NULL) {
        fputs ("File error",stderr); exit (1);
    }
    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    LONG lSize = ftell (pFile);
    fclose(pFile);


    std::list<NCDiagMMRecord> listRecord;

    DWORD dwRecSize = 0;
    collectFileRecord(eRecdID, listRecord, dwRecSize, fileName, lSize);

    // sort by GpsTime
    listRecord.sort(compareRecordByTick);


    for (std::list<NCDiagMMRecord>::iterator it=listRecord.begin(); it!=listRecord.end(); ++it) {
        printf("%02X%02X%02X\n", *((*it).getTickTime()), *((*it).getTickTime()+1), *((*it).getTickTime()+2));
    }


    listRecord.clear();

    return 0;
}



int test1 ()
{
#if 0
    std::list<NCDiagMMRecord> mylist;
    std::list<NCDiagMMRecord>::iterator it;

    BYTE byData[18] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};

    BYTE byGpsTime1[3] = {0x11,0x22,0x33};
    NCDiagMMRecord cRecord1(18);
    cRecord1.padGpsTime(byGpsTime1);
    cRecord1.padData(0, byData, 18);

    BYTE byGpsTime2[3] = {0x21,0x22,0x33};
    NCDiagMMRecord cRecord2(18);
    cRecord2.padGpsTime(byGpsTime2);
    cRecord2.padData(0, byData, 18);

    BYTE byGpsTime3[3] = {0x21,0x12,0x33};
    NCDiagMMRecord cRecord3(18);
    cRecord3.padGpsTime(byGpsTime3);
    cRecord3.padData(0, byData, 18);

    mylist.push_back (cRecord1);
    mylist.push_back (cRecord2);
    mylist.push_back (cRecord3);

    mylist.sort(compareRecord);

    //for (it=mylist.begin(); it!=mylist.end(); ++it) {
    //    printf("%0x %0x %0x\n", (*it).m_byGpsTime[0], (*it).m_byGpsTime[1], (*it).m_byGpsTime[2]);
    //}
#endif





    return 0;
}




int main()
{
    // test1();

    // test2
    NCDIAG_MMCollectFile cCollectFile;
    cCollectFile.collectFile();

    return 0;
}