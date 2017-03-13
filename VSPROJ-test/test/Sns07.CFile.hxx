#include <afx.h>
#include <iostream>
using namespace std;

#define NP_GPS_SNS_MEM_SIZE                 1024                ///< ���󥵤ΥХå����åץ���Υ�����
BYTE		m_byTempSnsMem[NP_GPS_SNS_MEM_SIZE];	// �ƥ�ݥ���� ����ѧ�� ����ǩ`��
WORD		m_byTempSnsMemCnt;		// �ƥ�ݥ���� ����ѧ�� ����ǩ`���å�����

WORD SNS_SIGNWORD;

//--------------read

/******************************************************************************
���I�����ťǩ`���É�Q�v�� Big Endian -> Little Endian
��������2Byte ���Ÿ�������
�����h��
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
���I�����󥵥Хå����åץǩ`����Хåե��طe���v��
��������
�����h�����󥵥Хå����åץǩ`����Хåե��طe���v��
******************************************************************************/
inline VOID PutSnsBkData(BYTE data)
{

	/* �Хåե���������򳬤������ */
	if (m_byTempSnsMemCnt >= NP_GPS_SNS_MEM_SIZE)  {
		/* ����`�Ěs��Ф�? */

		/* �Хåե��˥ǩ`����e���¤ǳ����ʤ��Τ��B��ƒi���� */
		return;
	}

	/* �����Υǩ`����Хåե��˷e�� */
	m_byTempSnsMem[m_byTempSnsMemCnt++] = data;
}

/******************************************************************************
���I�����󥵥Хå����åץǩ`����Хåե��طe���v��
��������
�����h�����󥵥Хå����åץǩ`����Хåե��طe���v��
******************************************************************************/
inline VOID PutSnsBkBlockData(
					   BYTE	*data,	/* �ǩ`���ؤΥݥ��� */
					   SHORT	len	/* �ǩ`���L */
					   )
{

	data += len;

	/* �ǩ`���L���R�귵�����Ф� */
	while (len)  {
		/* ����ǩ`���Υݥ���λ�ä򤺤餹 */
		data--;

		/* 1BYTE�֥ǩ`����e�� */
		PutSnsBkData(*data);

		/* �ǩ`���L��ǥ�����Ȥ������¤ǡ�0�ˤʤä��r���while()��i�������� */
		len--;
	}
}
inline VOID ConvertSnsBkDataToMem(VOID)
{
	/* SNS_SIGNWORD ������ */
	PutSnsBkBlockData((BYTE *)&SNS_SIGNWORD, 2);
}


/**  ���󥵥��걣��״�B�ж��åե饰  */
enum NPGpseHaveSnsMemEnum{
    NP_GPS_HAVE_SNS_MEM_OFF = 0,                    ///< ���󥵥��걣��ǩ`���o��
    NP_GPS_HAVE_SNS_MEM_1,                          ///< ���󥵥��걣��ǩ`�������Є�
    NP_GPS_HAVE_SNS_MEM_2                           ///< ���󥵥��걣��ǩ`�������Є�
} ;

/** GPS����`�Ěs������ */
struct NPGpsErrorInfoStr{
    BYTE    ErrorCount;                                             ///< ����`�΂���
    BYTE    ErrorNumber;                                            ///< ����`����
    SHORT   WeekNumber;                                             ///< ����`�k���L����
    DWORD   TimeOfWeek;                                             ///< ����`�k���r�� [sec] (GPS������)
    BYTE    DayOfWeek;                                              ///< ����`�k������
    BYTE    RtcTime[6];                                             ///< RTC�Εr�� �����Օr����
    LONG    Latitude;                                               ///< Latitude [1/256sec]
    LONG    Longitude;                                              ///< Longitude [1/256sec]
    SHORT   Altitude;                                               ///< Altitude [m]
    WORD    Velocity;                                               ///< Velocity [0.1 km/h]
    DWORD   Value;                                                  ///< ����΂�
    WORD    Lines;                                                  ///< ����`���k�������з���
    CHAR    Id[82];                                                 ///< VERSION�Ƕ��x�����ե����������Щ`����󷬺�
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