//#pragma pack(4)

#include <iostream>
#include <Windows.h>
using namespace std;

//#pragma pack(push, 4)
/** ���[���\���́iLOC�ւ̃��[���j */
struct NPGpsSnsMailStr{
    DWORD    DataStatusA;            ///< �X�e�[�^�XA
    DWORD    DataStatusB;            ///< �X�e�[�^�XB

    DOUBLE  Longitude;                          ///< �o�x [sec]
    DOUBLE  Latitude;                           ///< �ܓx [sec]
    FLOAT   DeltaLon;                           ///< �o�x�����ړ��� [sec]
    FLOAT   DeltaLat;                           ///< �ܓx�����ړ��� [sec]
    FLOAT   DeltaD;                             ///< �ړ��� [m]
	DOUBLE	Odometer;							//!< �I�h���[�^
    FLOAT   DeltaDir;                           ///< ���ʕω��� [deg]
    SHORT   Altitude;                           ///< ���x [m]
    SHORT   Heading;                            ///< ���� [360/65536 deg])
    WORD    Velocity;                           ///< ���x (0.1[km/h])
    SHORT   GsnsAngle;                          ///< �f�Z���T�[�X�Ίp
    CHAR    AccX;                               ///< �O������̉����x [0.1G]
    CHAR    AccY;                               ///< �������̉����x [0.1G]
    SHORT   GpsDirection;                       ///< GPS���� (��:0[deg],�����v���) [360/65536 deg]
    WORD    GpsVelocity;                        ///< GPS���x [0.1km/h]
    WORD    Laxis;                              ///< �덷�ȉ~���� [m]
    WORD    Saxis;                              ///< �덷�ȉ~�Z�� [m]
    SHORT   Angle;                              ///< �덷�����̌X�� [deg]
    BYTE    AdjPDOP;                            ///< PDOP�i2D���͍ŏI3D��VDOP���g�p���Čv�Z�j
    BYTE    HDOP;                               ///< HDOPx10 (max 255)
    BYTE    VDOP;                               ///< VDOPx10 (max 255)
    BYTE    TS;                                 ///< number of Tracking SV
    DWORD   SvNumber;                           ///< Tracking SV Number
    BYTE    SnsIncEstError;                     ///< �X�ΐ���␳�̌덷���ς����[DEG](Max 255)
    BYTE    Reserve;                            ///< ���U�[�u
    BYTE    GyroStatus;                         ///< �W���C���X�e�[�^�X
    BYTE    PulseStatus;                        ///< �����X�e�[�^�X
    BYTE    GsnsStatus;                         ///< �f�Z���T�[�X�e�[�^�X
    BYTE    SnsDirError;                        ///< ���ʌ덷 [0.1deg]) (max 25.5deg)
    BYTE    SnsMoveError;                       ///< �����덷 [0.1%]) (max 25.5 %)
    BYTE    SnsGsnsError;                       ///< �f�Z���T�덷 [0.1%]) (max 25.5 %)
	
} ;

//#pragma pop()

int main()
{
	cout << sizeof(NPGpsSnsMailStr) << endl;


	return 0;
}