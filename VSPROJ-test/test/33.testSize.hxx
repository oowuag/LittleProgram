//#pragma pack(4)

#include <iostream>
#include <Windows.h>
using namespace std;

//#pragma pack(push, 4)
/** [\¢ÌiLOCÖÌ[j */
struct NPGpsSnsMailStr{
    DWORD    DataStatusA;            ///< Xe[^XA
    DWORD    DataStatusB;            ///< Xe[^XB

    DOUBLE  Longitude;                          ///< ox [sec]
    DOUBLE  Latitude;                           ///< Üx [sec]
    FLOAT   DeltaLon;                           ///< oxûüÚ®Ê [sec]
    FLOAT   DeltaLat;                           ///< ÜxûüÚ®Ê [sec]
    FLOAT   DeltaD;                             ///< Ú®Ê [m]
	DOUBLE	Odometer;							//!< Ih[^
    FLOAT   DeltaDir;                           ///< ûÊÏ»Ê [deg]
    SHORT   Altitude;                           ///< x [m]
    SHORT   Heading;                            ///< ûÊ [360/65536 deg])
    WORD    Velocity;                           ///< ¬x (0.1[km/h])
    SHORT   GsnsAngle;                          ///< fZT[XÎp
    CHAR    AccX;                               ///< OãûüÌÁ¬x [0.1G]
    CHAR    AccY;                               ///< ¡ûüÌÁ¬x [0.1G]
    SHORT   GpsDirection;                       ///< GPSûÊ (:0[deg],½vñè) [360/65536 deg]
    WORD    GpsVelocity;                        ///< GPS¬x [0.1km/h]
    WORD    Laxis;                              ///< ë·È~·² [m]
    WORD    Saxis;                              ///< ë·È~Z² [m]
    SHORT   Angle;                              ///< ë··²ÌX« [deg]
    BYTE    AdjPDOP;                            ///< PDOPi2DÍÅI3DÌVDOPðgpµÄvZj
    BYTE    HDOP;                               ///< HDOPx10 (max 255)
    BYTE    VDOP;                               ///< VDOPx10 (max 255)
    BYTE    TS;                                 ///< number of Tracking SV
    DWORD   SvNumber;                           ///< Tracking SV Number
    BYTE    SnsIncEstError;                     ///< XÎùñâ³Ìë·©ÏàèÊ[DEG](Max 255)
    BYTE    Reserve;                            ///< U[u
    BYTE    GyroStatus;                         ///< WCXe[^X
    BYTE    PulseStatus;                        ///< £Xe[^X
    BYTE    GsnsStatus;                         ///< fZT[Xe[^X
    BYTE    SnsDirError;                        ///< ûÊë· [0.1deg]) (max 25.5deg)
    BYTE    SnsMoveError;                       ///< £ë· [0.1%]) (max 25.5 %)
    BYTE    SnsGsnsError;                       ///< fZTë· [0.1%]) (max 25.5 %)
	
} ;

//#pragma pop()

int main()
{
	cout << sizeof(NPGpsSnsMailStr) << endl;


	return 0;
}