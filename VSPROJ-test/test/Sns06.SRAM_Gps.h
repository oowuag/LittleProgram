#include <Windows.h>

typedef BOOL NP_BOOL;
#define NP_TRUE TRUE
#define NP_FALSE FALSE

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

/***  GPS・センサステータス共用体Ａ (LOC)  ***/
typedef union {
    struct {
	DWORD	GpsOk:1;		/* GPSデータ使用可能判断 (1:OK)                    */
	DWORD	GyroOk:1;		/* ジャイロデータ使用可能判断 (1:OK)               */
	DWORD	PulseOk:1;		/* パルスデータ使用可能判断 (1:OK)                 */
	DWORD	GsnsOk:1;		/* Gセンサデータ使用可能判断 (1:OK)                */
	DWORD	Differential:1;		/* GPSディファレンシャル (1:differntial)           */
	DWORD	GpsDimension:2;		/* GPS測位次元 (0:0D 1:1D(誤差大) 2:2D 3:3D)       */
	DWORD	DirOk:1;		/* 方位使用可能判断 (1:OK)                         */
	DWORD	DirSource:1;		/* 方位の出力元 (0:Gyro 1:GPS)                     */
	DWORD	VelOk:1;		/* 速度使用可能判断 (1:OK)                         */
	DWORD	VelSource:2;		/* 速度の出力元   (0:Pulse 1:GPS 2:G-Sensor)       */
	DWORD	MovSource:2;		/* 移動量の出力元 (0:Pulse 1:GPS 2:G-Sensor)       */
	DWORD	LR:2;			/* 右左折判断 (1:右折 2:左折)                      */
	DWORD	Inclination:2;		/* 傾斜ステータス (0:判断不能 1:UP 2:DOWN 3:LEVEL) */
	DWORD	Back:1;			/* バック線接続によるバックを検出 (0:前進 1:後退)  */
	DWORD	GpsNoSignal:1;		/* GPS衛星の電波を全く受信できていない             */
	DWORD	GpsMultiPath:1;		/* GPS測位データにマルチパスの影響を検出 (1:検出)  */
	DWORD	PulseError:1;		/* 車速パルス異常 (1:異常)                         */
	DWORD	DragonNG:1;		/* DRAGON chipとの接続異常 (0:正常, 1:異常)        */
	DWORD	UpdateInterval:2;	/* 測位周期 (0:2Hz 1:4Hz 2:5Hz 3:10Hz)             */
	DWORD	ReqPosMail:1;		/* 現在のデータ送出状態ステータス (1:out)          */
	DWORD	GpsSA:1;		/* SAフラグ (0:SA無し 1:SA有り)                    */
	DWORD	WeavingAlarm:2;		/* ふらつき警告 (0:警告なし 1～3:警告レベル1～3)   */
	DWORD	CautionStatus:1;	/* GPS、センサ系エラー情報 (0:正常, 1:異常)        */
	DWORD	GpsDelayCorrFunc:1;	/* GPS測位ディレイ補正機能搭載フラグ(0:機能な
し 1:機能あり) */
	DWORD	GpsDelayCorrect:1;	/* GPS測位ディレイ補正フラグ(0:補正なし 1:補正
あり)*/
    } sCondition;
    DWORD	AllStatus;		/* 全体のステータス                                */
} uDatStatA;

/***  GPS・センサステータス共用体Ｂ (Debug)  ***/
typedef union {
    struct {
	DWORD	GpsUpdate:1;		/* GPSデータが更新されている (1:更新)                */
	DWORD	GpsBusy:1;		/* GPSデータを計算中 (1:計算中)                      */
	DWORD	ClockHold:1;		/* 少衛星測位中 (1:CHM)                              */
	DWORD	MMAidGps:1;		/* GPS遮断復帰位置にMM位置を使用 (1:使用)            */
	DWORD	SnsAidGps:1;		/* GPS遮断復帰位置にセンサ移動距離を使用 (1:使用)    */
	DWORD	SnsAidGpsFilt:1;	/* GPS位置フィルタにセンサ移動距離を使用 (1:使用)    */
	DWORD	SnsAidGpsAlt:1;		/* GPSの2D高度に3Dセンサ測定値を使用 (1:使用)        */
	DWORD	BackLineOk:1;		/* バック線接続有無 (1:有り)                         */
	DWORD	BackSource:2;		/* バック判定の判断元 (0:無 1:バック線 2:G-Sensor)   */
	DWORD	GyroDataNG:1;		/* ジャイロデータ誤差大 (1:誤差大)                   */
	DWORD	PulseDataNG:1;		/* パルスデータ誤差大 (1:誤差大)                     */
	DWORD	GsnsDataNG:1;		/* Gセンサデータ誤差大 (1:誤差大)                    */
	DWORD	GpsLog:1;		/* GPSデータロギング中                               */
	DWORD	SnsLog:1;		/* SNSデータロギング中                               */
	DWORD	TurnLightR:1;		/* 右ウインカー信号あり (車線変更検出用)             */
	DWORD	TurnLightL:1;		/* 左ウインカー信号あり (車線変更検出用)             */
	DWORD	NdPg1:1;		/* ND-PG1フラグ (1:ND-PG1)                           */
	DWORD	LowSpeedNg:1;		/* 低速車速パルスNG車両 (1:低速NG)                   */
	DWORD	ForwardLogic:1;		/* バック線前進論理(0:前進論理Low 1:前進論理 High)   */
	DWORD	PulseNGLevel:2;		/* 距離誤差レベル(0:通常 1:誤差小 2:誤差中 3:誤差大) */
	DWORD	ReserveBits:2;		/* 残りのビット                                      */
	DWORD	DebugStatus:8;		/* デバッグ用の汎用ステータス                        */
    } sCondition;
    DWORD	AllStatus;		/* 全体のステータス                                  */
} uDatStatB;

/*** GPS・センサ出力データ構造体 ***/
struct sGpsSnsMail{
    uDatStatA	uDataStatusA;		/* ステータス共用体Ａ                              */
    uDatStatB	uDataStatusB;		/* ステータス共用体Ｂ                              */
    DOUBLE	Longitude;		/* 経度 [sec]                                      */
    DOUBLE	Latitude;		/* 緯度 [sec]                                      */
    FLOAT	DeltaLon;		/* 経度方向移動量 [sec]                            */
    FLOAT	DeltaLat;		/* 緯度方向移動量 [sec]                            */
    FLOAT	DeltaD;			/* 移動量 [m]                                      */
    FLOAT	DeltaDir;		/* 方位変化量 [deg]                                */
    SHORT	Altitude;		/* 高度 [m]                                        */
    SHORT	Heading;		/* 方位 [360/65536 deg] (東:0[deg] 反時計回り)     */
    WORD	Velocity;		/* 速度 [0.1km/h]                                  */
    SHORT	GsnsAngle;		/* Ｇセンサー傾斜角 [0.01 deg]                     */
    CHAR	AccX;			/* 前後方向の加速度 [0.1G]                         */
    CHAR	AccY;			/* 横方向の加速度 [0.1G]                           */
    SHORT	GpsDirection;		/* GPS方位 [360/65536 deg] (東:0[deg] 反時計回り)  */
    WORD	GpsVelocity;		/* GPS速度 [0.1km/h]                               */
    WORD	Laxis;			/* 誤差楕円長軸 [m]                                */
    WORD	Saxis;			/* 誤差楕円短軸 [m]                                */
    SHORT	Angle;			/* 誤差長軸の傾き [deg] (東:0[deg] 反時計回り)     */
    BYTE	AdjPDOP;		/* PDOP（2D時は最終3DのVDOPを使用して計算）        */
    BYTE	HDOP;			/* HDOPx10 (max 255)                               */
    BYTE	VDOP;			/* VDOPx10 (max 255)                               */
    BYTE	TS;			/* Number of Tracking SV                           */
    DWORD	SvNumber;		/* Tracking SV Number (bit0:PRN1, ... bit31:PRN32) */
    BYTE	SnsIncEstError;		/* 傾斜旋回補正の誤差見積もり量[DEG](Max 255)      */
    BYTE	Reserve;		/* リザーブ                                        */
    BYTE	GyroStatus;		/* ジャイロステータス                              */
    BYTE	PulseStatus;		/* 距離ステータス                                  */
    BYTE	GsnsStatus;		/* Ｇセンサーステータス                            */
    BYTE	SnsDirError;		/* 方位誤差 [0.1deg]) (max 25.5deg)                */
    BYTE	SnsMoveError;		/* 距離誤差 [0.1%]) (max 25.5 %)                   */
    BYTE	SnsGsnsError;		/* Ｇセンサ誤差 [0.1%]) (max 25.5 %)               */
} ;

/** 各GPS衛星の情報 */
struct NPGpsSatDataStr{
    BYTE    SvPrn;                                                  ///< 衛星番号
    BYTE    SvStatus;                                               ///< 衛星の捕捉状態
    BYTE    Elevation;                                              ///< 仰角 [LSB 90/256 deg]
    BYTE    Azimuth;                                                ///< 方位角 [LSB 360/256 deg]
    SHORT   Doppler;                                                ///< ドップラー周波数 [LSB 1 Hz]
    SHORT   SearchRange;                                            ///< サーチ幅 [LSB 1 Hz]
    BYTE    SvLevel;                                                ///< 受信レベル [LSB 1 C/N0]
} ;

/**  構造体で使うデータサイズ  */
#define NP_GPS_ERROR_INFO_CNT               20                  ///< エラー履歴の保持数
#define NP_GPS_SNS_MEM_SIZE                 1024                ///< センサのバックアップメモリのサイズ
#define NP_GPS_ERR2_RCV_MAX                 10                  ///< 受信するエラー履歴（詳細）の構造体配列数
#define NP_GPS_SAT_NUM_MAX                  32                  ///< 衛星情報最大数


/** GPS衛星情報構造体 */
struct NPGpsSatStatusStr{
    NPGpsSatDataStr CurrSat[NP_GPS_SAT_NUM_MAX];                    ///< 32衛星分の衛星情報
    BYTE            TrackSat;                                       ///< 受信機がサーチまたは捕捉している衛星数
    BYTE            reserve[3];
} ;

/** API用共有メモリの構造体 */
struct NPGpsStoreDataStatusStr{
    NPGpsErrorInfoStr       sGpsError[NP_GPS_ERROR_INFO_CNT];       ///< エラー履歴保存用
    NPGpsSatStatusStr       sSatStatus;                             ///< 衛星情報
    DWORD                   dwStoreDataStatus;                      ///< バッファ参照データ格納フラグ
    NP_BOOL                 bIsGps;                                 ///< GPS接続OK/NGフラグ
    NP_BOOL                 bFileaccessOK;                          ///< ファイルアクセスOKフラグ
    NP_BOOL                 bGpsGetSnsMail;                         ///< SnsMailデータ有効フラグ
    NP_BOOL                 bGpsReceiveErrorInf;                    ///< エラー履歴データ有効フラグ
} ;


/** 時刻データ構造体 */
struct NPGpsTimeInfoStr{
    BYTE    CurrTime[8];                                            ///< 現在時刻   (BCD 表示データ)
    DWORD   MsecOfWeek;                                             ///< 現在週時刻[msec] (Sunday 0:00 -)
    WORD    GpsWeek;                                                ///< 現在週 (1999.8.22 -)
    BYTE    Status;                                                 ///< 返答時刻状態
} ; 

/** メールステータス共用体A (LOC) */
typedef union {
    struct {
        DWORD   GpsOk:1;                        ///< GPSデータ使用可能判断 (1:OK)
        DWORD   GyroOk:1;                       ///< ジャイロデータ使用可能判断 (1:OK)
        DWORD   PulseOk:1;                      ///< パルスデータ使用可能判断 (1:OK)
        DWORD   GsnsOk:1;                       ///< Gセンサデータ使用可能判断 (1:OK)
        DWORD   Differential:1;                 ///< GPSディファレンシャル (1:differntial)
        DWORD   GpsDimension:2;                 ///< GPS測位次元 (00:0D, 01:1D, 10:2D, 11:3D)
        DWORD   DirOk:1;                        ///< 方位使用可能判断 (1:OK)
        DWORD   DirSource:1;                    ///< 方位の出力元 (0:Gyro, 1:GPS)
        DWORD   VelOk:1;                        ///< 速度使用可能判断 (1:OK)
        DWORD   VelSource:2;                    ///< 速度の出力元 (0:Pulse, 1:GPS, 2:G-Sensor)
        DWORD   MovSource:2;                    ///< 移動量の出力元 (0:Pulse, 1:GPS, 2:G-Sensor)
        DWORD   LR:2;                           ///< 右左折判断 (1:右折, 2:左折)
        DWORD   Inclination:2;                  ///< 傾斜ステータス (0:傾斜判断不能, 1:UP, 2:DOWN, 3:LEVEL)
        DWORD   Back:1;                         ///< バック線接続によるバックを検出 (0:前進, 1:後退)
        DWORD   GpsNoSignal:1;                  ///< GPS衛星の電波を全く受信できていない
        DWORD   GpsMultiPath:1;                 ///< GPS測位データにマルチパスの影響を検出 (1:検出)
        DWORD   PulseError:1;                   ///< 車速パルス異常 (1:異常)
        DWORD   DragonNG:1;                     ///< GPS chipとの接続異常 (0:正常, 1:異常)
        DWORD   UpdateInterval:2;               ///< 測位周期 (0:2Hz, 1:4Hz, 2:5Hz, 3:10Hz)
        DWORD   ReqPosMail:1;                   ///< 現在のデータ送出状態ステータス (1:out)
        DWORD   GpsSA:1;                        ///< SAフラグ（0:SAなし, 1:SAあり）
        DWORD   WeavingAlarm:2;                 ///< ふらつき警告 (0:警告なし 1～3:警告レベル1～3)
        DWORD   CautionStatus:1;                ///< GPS、センサ系エラー情報 (0:正常, 1:異常)
        DWORD   GpsDelayCorrFunc:1;             ///< GPS測位ディレイ補正機能搭載フラグ(0:機能なし 1:機能あり)
        DWORD   GpsDelayCorrect:1;              ///< GPS測位ディレイ補正フラグ(0:補正なし 1:補正あり)
    } Condition;
    DWORD       AllStatus;                      ///< 全体のステータス
} NPGpsDatStatAUni;

         
/** メールステータス共用体B (Debug) */
typedef union {
    struct {
        DWORD   GpsUpdate:1;                    ///< GPSデータが更新されている (1:更新)
        DWORD   GpsBusy:1;                      ///< GPSデータを計算中 (1:計算中)
        DWORD   ClockHold:1;                    ///< 少衛星測位中 (1:CHM)
        DWORD   MMAidGps:1;                     ///< GPS遮断復帰位置にMM位置を使用 (0:未使用, 1:使用)
        DWORD   SnsAidGps:1;                    ///< GPS遮断復帰位置にセンサ移動距離を使用 (0:未使用, 1:使用)
        DWORD   SnsAidGpsFilt:1;                ///< GPS位置フィルタにセンサ移動距離を使用 (0:未使用, 1:使用)
        DWORD   SnsAidGpsAlt:1;                 ///< GPSの2D高度に3Dセンサ測定値を使用 (0:未使用, 1:使用)
        DWORD   BackLineOk:1;                   ///< バック線接続有無 (1:有り)
        DWORD   BackSource:2;                   ///< バック判定の判断元 (0:判断できず、1:バック線, 2:G-Sensor)
        DWORD   GyroDataNG:1;                   ///< ジャイロデータ誤差大 (1:誤差大)
        DWORD   PulseDataNG:1;                  ///< パルスデータ誤差大 (1:誤差大)
        DWORD   GsnsDataNG:1;                   ///< Gセンサデータ誤差大 (1:誤差大)
        DWORD   GpsLog:1;                       ///< GPSデータロギング中
        DWORD   SnsLog:1;                       ///< SNSデータロギング中
        DWORD   TurnLightR:1;                   ///< 右ウインカー信号あり
        DWORD   TurnLightL:1;                   ///< 左ウインカー信号あり
        DWORD   NdPg1:1;                        ///< ND-PG1フラグ (1:ND-PG1)
        DWORD   LowSpeedNg:1;                   ///< 低速車速パルスNG車両 (1:低速NG)
        DWORD   ForwardLogic:1;                 ///< バック線前進論理(0:前進論理Low 1:前進論理 High)
	DWORD	PulseNGLevel:2;	               	///< 距離誤差レベル(0:通常 1:誤差小 2:誤差中 3:誤差大)
        DWORD   ReserveBits:2;                  ///< 残りのビット

        DWORD   DebugStatus:8;                  ///< デバッグ用の汎用ステータス
    } Condition;
    DWORD       AllStatus;                      ///< 全体のステータス
} NPGpsDatStatBUni;


/** メール構造体（LOCへのメール） */
struct NPGpsSnsMailStr{
    NPGpsDatStatAUni    DataStatusA;            ///< ステータスA
    NPGpsDatStatBUni    DataStatusB;            ///< ステータスB

    DOUBLE  Longitude;                          ///< 経度 [sec]
    DOUBLE  Latitude;                           ///< 緯度 [sec]
    FLOAT   DeltaLon;                           ///< 経度方向移動量 [sec]
    FLOAT   DeltaLat;                           ///< 緯度方向移動量 [sec]
    FLOAT   DeltaD;                             ///< 移動量 [m]
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

/** GPS位置構造体 */
struct NPGpsPosDataStr {
    WORD    StatusCode;                                             ///< 測位ステータス
    WORD    TrackSat;                                               ///< 測位使用可能衛星数
    DWORD   SvNum;                                                  ///< 捕捉衛星番号
    LONG    Latitude;                                               ///< 緯度 [LSB 1/256 s]
    LONG    Longitude;                                              ///< 経度 [LSB 1/256 s]
    SHORT   Altitude;                                               ///< 高度 [LSB 1 m]
    SHORT   Velocity;                                               ///< 水平方向の速度 [LSB 0.1 km/h]
    SHORT   Heading;                                                ///< 水平方向速度方位 [LSB 360/65536 deg] 北:0 時計回り
    SHORT   Climb;                                                  ///< 垂直方向の速度 [LSB 0.1 km/h]
    BYTE    Hdop;                                                   ///< HDOP水平方向精度 [LSB 0.1]
    BYTE    Vdop;                                                   ///< VDOP高度方向精度 [LSB 0.1]
    WORD    Laxis;                                                  ///< 誤差楕円長軸 [LSB 1 m]
    WORD    Saxis;                                                  ///< 誤差楕円短軸 [LSB 1 m]
    WORD    LaDir;                                                  ///< 誤差長軸の傾き [LSB 360/256 deg] 北:0 時計回り
    BYTE    VelAcc;                                                 ///< 速度の精度 [LSB 0.1 km/h]
    BYTE    DirAcc;                                                 ///< 方位の精度 [LSB 360/65536 deg]
    BYTE    LocalTime[6];                                           ///< 現在時刻データ [BCD] YY MM DD HH MM SS
    BYTE    FixDelay;                                               ///< 測位時刻からの遅れ [LSB 0.1 sec]
} ;

/** 共有メモリのPoseidonデータ */
struct NPGpsPoseidonDataStr{
    NPGpsSnsMailStr         sSnsMail;                               ///< SNSメール
    NPGpsPosDataStr         sPosData;                               ///< POSデータ
    NPGpsTimeInfoStr        sTimeInfo;                              ///< GPS時刻
} ;
