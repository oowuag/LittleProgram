#include <afx.h>
#include <iostream>
using namespace std;

#define NP_GPS_SNS_MEM_SIZE                 1024                ///< センサのバックアップメモリのサイズ
BYTE		m_byTempSnsMem[NP_GPS_SNS_MEM_SIZE];	// テンポラリ喘 センサ僥�� 隠贋デ�`タ
WORD		m_byTempSnsMemCnt;		// テンポラリ喘 センサ僥�� 隠贋デ�`タ喘カウンタ

WORD SNS_SIGNWORD;

//--------------read

/******************************************************************************
‐�I尖／鞭佚デ�`タ喘���Q�v方 Big Endian -> Little Endian
‐���｡�2Byte 憲催原き屁方
‐盾�h／
******************************************************************************/
inline SHORT  Byte2SWord(BYTE *data)
{
	SHORT     i = 0;
	BYTE    *ip = NULL;

	ip = (BYTE *)&i;
	*ip++ = data[1];
	*ip = data[0];
	return (i);
}

inline WORD Byte2UWord(BYTE *data)
{
	return ((WORD)Byte2SWord(data));
}

VOID ConvertSnsMemToSnsBkData(VOID)
{
	WORD	data_cnt = 0;
	SNS_SIGNWORD		= Byte2UWord(&m_byTempSnsMem[data_cnt]); data_cnt+=2;	/* SENSOR SIGNWORD */
}

//--------------write

/******************************************************************************
‐�I尖／センサバックアップデ�`タをバッファへ�eむ�v方
‐���｡�
‐盾�h／センサバックアップデ�`タをバッファへ�eむ�v方
******************************************************************************/
inline VOID PutSnsBkData(BYTE data)
{

	/* バッファの恷寄方を階える��栽 */
	if (m_byTempSnsMemCnt >= NP_GPS_SNS_MEM_SIZE)  {
		/* エラ�`堕�sを火す? */

		/* バッファにデ�`タを�eむ並で竃栖ないので�Bめて�iける */
		return;
	}

	/* 哈方のデ�`タをバッファに�eむ */
	m_byTempSnsMem[m_byTempSnsMemCnt++] = data;
}

/******************************************************************************
‐�I尖／センサバックアップデ�`タをバッファへ�eむ�v方
‐���｡�
‐盾�h／センサバックアップデ�`タをバッファへ�eむ�v方
******************************************************************************/
inline VOID PutSnsBkBlockData(
					   BYTE	*data,	/* デ�`タへのポインタ */
					   SHORT	len	/* デ�`タ�L */
					   )
{

	data += len;

	/* デ�`タ�L蛍�Rり卦しを佩う */
	while (len)  {
		/* 旗秘デ�`タのポインタ了崔をずらす */
		data--;

		/* 1BYTE蛍デ�`タを�eむ */
		PutSnsBkData(*data);

		/* デ�`タ�Lをデクリメントさせる並で、0になった�r泣でwhile()を�iけさせる */
		len--;
	}
}
inline VOID ConvertSnsBkDataToMem(VOID)
{
	/* SNS_SIGNWORD を僕佚 */
	PutSnsBkBlockData((BYTE *)&SNS_SIGNWORD, 2);
}


/**  センサメモリ隠贋彜�B登協喘フラグ  */
enum NPGpseHaveSnsMemEnum{
    NP_GPS_HAVE_SNS_MEM_OFF = 0,                    ///< センサメモリ隠贋デ�`タ�oし
    NP_GPS_HAVE_SNS_MEM_1,                          ///< センサメモリ隠贋デ�`タ１が嗤��
    NP_GPS_HAVE_SNS_MEM_2                           ///< センサメモリ隠贋デ�`タ２が嗤��
} ;

/** GPSエラ�`堕�s��夛悶 */
struct NPGpsErrorInfoStr{
    BYTE    ErrorCount;                                             ///< エラ�`の��方
    BYTE    ErrorNumber;                                            ///< エラ�`桑催
    SHORT   WeekNumber;                                             ///< エラ�`�k伏�L桑催
    DWORD   TimeOfWeek;                                             ///< エラ�`�k伏�r震 [sec] (GPSタイム)
    BYTE    DayOfWeek;                                              ///< エラ�`�k伏袗晩
    BYTE    RtcTime[6];                                             ///< RTCの�r震 定埖晩�r蛍昼
    LONG    Latitude;                                               ///< Latitude [1/256sec]
    LONG    Longitude;                                              ///< Longitude [1/256sec]
    SHORT   Altitude;                                               ///< Altitude [m]
    WORD    Velocity;                                               ///< Velocity [0.1 km/h]
    DWORD   Value;                                                  ///< 販吭の��
    WORD    Lines;                                                  ///< エラ�`が�k伏した佩桑催
    CHAR    Id[82];                                                 ///< VERSIONで協�xしたファイル兆、バ�`ジョン桑催
} ;

inline void func()
{
	while (1)
	{
		func();
	}
}

int main()
{



	// constructing these file objects doesn't open them
	CFile writeFile;
	CFile readFile;
	// we'll use a CFileException object to get error information
	CFileException ex;

	// open the source file for reading
	if (!writeFile.Open("D:\\writeFileSns.txt",
		CFile::modeWrite | CFile::modeCreate, &ex))
	{
		// complain if an error happened
		// no need to delete the ex object

		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		cout << "Couldn't open source file: ";
		cout << szError;
		return 1;
	}

	memset(m_byTempSnsMem, 0, NP_GPS_SNS_MEM_SIZE);
	SNS_SIGNWORD = 0x3456;
	ConvertSnsBkDataToMem();	
	writeFile.Write(m_byTempSnsMem, 2);
	WORD temp1 = SNS_SIGNWORD;
	writeFile.Write(&temp1, 2);
	writeFile.Close();

	//return 0;

	if (!readFile.Open("D:\\writeFileSns.txt", CFile::modeRead, &ex))
	{
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		cout << "Couldn't open source file: ";
		cout << szError;

		readFile.Close();
		return 1;
	}

	memset(m_byTempSnsMem, 0, NP_GPS_SNS_MEM_SIZE);
	readFile.Read(m_byTempSnsMem, 2);
	ConvertSnsMemToSnsBkData();
	WORD temp2 = 0;
	readFile.Read(&temp2,2);
	readFile.Close(); //same effect

	cout << SNS_SIGNWORD << endl;
	cout << temp2 << endl;


	cout << sizeof(NPGpseHaveSnsMemEnum) << endl;
	cout << sizeof(NPGpsErrorInfoStr) << endl;

	func();

	return 0;
}