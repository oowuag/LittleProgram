#include <afx.h>
#include <iostream>
using namespace std;

#define NP_GPS_SNS_MEM_SIZE                 1024                ///< センサのバックアップメモリのサイズ
BYTE		m_byTempSnsMem[NP_GPS_SNS_MEM_SIZE];	// テンポラリ用 センサ学習 保存データ
WORD		m_byTempSnsMemCnt;		// テンポラリ用 センサ学習 保存データ用カウンタ

WORD SNS_SIGNWORD;

//--------------read

/******************************************************************************
【処理】受信データ用変換関数 Big Endian -> Little Endian
【戻値】2Byte 符号付き整数
【解説】
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
【処理】センサバックアップデータをバッファへ積む関数
【戻値】
【解説】センサバックアップデータをバッファへ積む関数
******************************************************************************/
inline VOID PutSnsBkData(BYTE data)
{

	/* バッファの最大数を超える場合 */
	if (m_byTempSnsMemCnt >= NP_GPS_SNS_MEM_SIZE)  {
		/* エラー履歴を残す? */

		/* バッファにデータを積む事で出来ないので諦めて抜ける */
		return;
	}

	/* 引数のデータをバッファに積む */
	m_byTempSnsMem[m_byTempSnsMemCnt++] = data;
}

/******************************************************************************
【処理】センサバックアップデータをバッファへ積む関数
【戻値】
【解説】センサバックアップデータをバッファへ積む関数
******************************************************************************/
inline VOID PutSnsBkBlockData(
					   BYTE	*data,	/* データへのポインタ */
					   SHORT	len	/* データ長 */
					   )
{

	data += len;

	/* データ長分繰り返しを行う */
	while (len)  {
		/* 代入データのポインタ位置をずらす */
		data--;

		/* 1BYTE分データを積む */
		PutSnsBkData(*data);

		/* データ長をデクリメントさせる事で、0になった時点でwhile()を抜けさせる */
		len--;
	}
}
inline VOID ConvertSnsBkDataToMem(VOID)
{
	/* SNS_SIGNWORD を送信 */
	PutSnsBkBlockData((BYTE *)&SNS_SIGNWORD, 2);
}


/**  センサメモリ保存状態判定用フラグ  */
enum NPGpseHaveSnsMemEnum{
    NP_GPS_HAVE_SNS_MEM_OFF = 0,                    ///< センサメモリ保存データ無し
    NP_GPS_HAVE_SNS_MEM_1,                          ///< センサメモリ保存データ１が有効
    NP_GPS_HAVE_SNS_MEM_2                           ///< センサメモリ保存データ２が有効
} ;

/** GPSエラー履歴構造体 */
struct NPGpsErrorInfoStr{
    BYTE    ErrorCount;                                             ///< エラーの個数
    BYTE    ErrorNumber;                                            ///< エラー番号
    SHORT   WeekNumber;                                             ///< エラー発生週番号
    DWORD   TimeOfWeek;                                             ///< エラー発生時刻 [sec] (GPSタイム)
    BYTE    DayOfWeek;                                              ///< エラー発生曜日
    BYTE    RtcTime[6];                                             ///< RTCの時刻 年月日時分秒
    LONG    Latitude;                                               ///< Latitude [1/256sec]
    LONG    Longitude;                                              ///< Longitude [1/256sec]
    SHORT   Altitude;                                               ///< Altitude [m]
    WORD    Velocity;                                               ///< Velocity [0.1 km/h]
    DWORD   Value;                                                  ///< 任意の値
    WORD    Lines;                                                  ///< エラーが発生した行番号
    CHAR    Id[82];                                                 ///< VERSIONで定義したファイル名、バージョン番号
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