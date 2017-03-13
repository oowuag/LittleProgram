//#pragma pack(4)

#include <iostream>
#include <Windows.h>
using namespace std;

//#pragma pack(push, 4)
/** メール構造体（LOCへのメール） */
struct NPGpsSnsMailStr{
    DWORD    DataStatusA;            ///< ステータスA
    DWORD    DataStatusB;            ///< ステータスB

    DOUBLE  Longitude;                          ///< 経度 [sec]
    DOUBLE  Latitude;                           ///< 緯度 [sec]
    FLOAT   DeltaLon;                           ///< 経度方向移動量 [sec]
    FLOAT   DeltaLat;                           ///< 緯度方向移動量 [sec]
    FLOAT   DeltaD;                             ///< 移動量 [m]
	DOUBLE	Odometer;							//!< オドメータ
    FLOAT   DeltaDir;                           ///< 方位変化量 [deg]
    SHORT   Altitude;                           ///< 高度 [m]
    SHORT   Heading;                            ///< 方位 [360/65536 deg])
    WORD    Velocity;                           ///< 速度 (0.1[km/h])
    SHORT   GsnsAngle;                          ///< Ｇセンサー傾斜角
    CHAR    AccX;                               ///< 前後方向の加速度 [0.1G]
    CHAR    AccY;                               ///< 横方向の加速度 [0.1G]
    SHORT   GpsDirection;                       ///< GPS方位 (東:0[deg],反時計回り) [360/65536 deg]
    WORD    GpsVelocity;                        ///< GPS速度 [0.1km/h]
    WORD    Laxis;                              ///< 誤差楕円長軸 [m]
    WORD    Saxis;                              ///< 誤差楕円短軸 [m]
    SHORT   Angle;                              ///< 誤差長軸の傾き [deg]
    BYTE    AdjPDOP;                            ///< PDOP（2D時は最終3DのVDOPを使用して計算）
    BYTE    HDOP;                               ///< HDOPx10 (max 255)
    BYTE    VDOP;                               ///< VDOPx10 (max 255)
    BYTE    TS;                                 ///< number of Tracking SV
    DWORD   SvNumber;                           ///< Tracking SV Number
    BYTE    SnsIncEstError;                     ///< 傾斜旋回補正の誤差見積もり量[DEG](Max 255)
    BYTE    Reserve;                            ///< リザーブ
    BYTE    GyroStatus;                         ///< ジャイロステータス
    BYTE    PulseStatus;                        ///< 距離ステータス
    BYTE    GsnsStatus;                         ///< Ｇセンサーステータス
    BYTE    SnsDirError;                        ///< 方位誤差 [0.1deg]) (max 25.5deg)
    BYTE    SnsMoveError;                       ///< 距離誤差 [0.1%]) (max 25.5 %)
    BYTE    SnsGsnsError;                       ///< Ｇセンサ誤差 [0.1%]) (max 25.5 %)
	
} ;

//#pragma pop()

int main()
{
	cout << sizeof(NPGpsSnsMailStr) << endl;


	return 0;
}