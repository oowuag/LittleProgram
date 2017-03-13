
#include <stdio.h>
#include <Windows.h>

#include <iostream>
using namespace std;


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

int getx()
{
	return 3;
}

int main(void)
{
	//NPGpsDatStatAUni xx = {0};
	//xx.Condition.DirSource = 5;

	//int x = 3;
	//switch(x) 
	//{
	//case 1+2:
	//	cout << "1+2" << endl; 
	//	break;
	//}

	DWORD n = 3;
	while(1)
	{
		n--;
		cout << n << endl; 
	}

	return 0;
}

//f==1.354370
