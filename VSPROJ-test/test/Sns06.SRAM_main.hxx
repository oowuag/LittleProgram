#include "Sns06.SRAM.hxx"

#include <iostream>
using namespace std;

#define COMP_OPT_FOR_MACHINE
const CHAR  VERSION[] = "$Id: NPSnsHandlerBase.cpp 4009 2009-06-29 09:15:39Z m-hashimoto $";


/******************************************************************************
【処理】SRAMセンサバックアップデータコピー関数
【戻値】なし
【解説】SRAM上のセンサバックアップデータをコピーする処理関数
******************************************************************************/
VOID NPSnsHandlerBase::CopySnsBkSRAM(BYTE sram_num)
{
	/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)
#else	/* !COMP_OPT_FOR_SNS_SIMULATOR */

	/**********************************************************************
	【注意事項】
	新規追加メンバは、本関数内では、必ず0で初期化すること
	(追加メンバ用の初期化関数は利用しないこと！！)
	これにより、センサバックアップデータのCSチェック時にCSが一致する

	本処理では、以下のことを前提として構築された処理である。
	・メンバの削除が合った場合、同型のリザーブデータとして定義される
	・基本部分までのメンバは変更がない
	(SRAM管理番号のアドレスが変更となった場合、対応できない）

	【追記事項】
	もしメンバの削除を行った際には、ポインタの調整を行うこと
	メンバの削除後のCSは、保存時のCSとは異るため、CSの調整も行うこと
	**********************************************************************/

	WORD	data_cnt = 0;	/* センサ学習データ保存バッファのインデックス */

	INT	i = 0;				/* ループカウンタ */
	INT	j = 0;				/* ループカウンタ */
	BYTE    sns_sram[NP_SNS_SECTION_AREA];	/* SRAM用バッファ */
	//NPSramData  cSram;				// SRAMアクセスクラス

	sSnsLearnStatus	*bk_l = NULL;		/* センサ初期学習状態用構造体 */
	sSensorBackupBasic	*bk_b = NULL;		/* センサバックアップ変数構造体 基本部分へのポインタ */
	sSensorBackupExtension	*bk_e = NULL;	/* センサバックアップ変数構造体 拡張部分へのポインタ */

	/* データカウンタの初期化 */
	data_cnt = 0;
	memset(&sns_sram[0], 0, NP_SNS_SECTION_AREA);

	///* Aセクション利用時 */
	//if (sram_num == NP_SNS_A_SECTION)  {
	//	/* Aセクションを読み込み */
	//	if ((cSram.Read(NP_SRAM_AREA_SNS, SNS_LEARN_BK_DATA_A , sns_sram, static_cast< DWORD >( AdjSnsMemSize ))) == NP_FALSE )  {
	//		// 失敗
	//		// システムとして動作できないので、エラーログを残す
	//		NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//	}
	//}
	///* Bセクション利用時 */
	//else {
	//	/* Bセクションを読み込み */
	//	if ((cSram.Read(NP_SRAM_AREA_SNS, SNS_LEARN_BK_DATA_B , sns_sram, static_cast< DWORD >( AdjSnsMemSize ))) == NP_FALSE)  {
	//		// 失敗
	//		// システムとして動作できないので、エラーログを残す
	//		NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//	}
	//}

	/*** データのセット処理 ***/

	memcpy(&SNS_SIGNWORD, &sns_sram[data_cnt], 2); data_cnt+=2;		/* SENSOR SIGNWORD */
	data_cnt+=2;	/* アライメント調整分 */
	memcpy(&SnsSRamNow, &sns_sram[data_cnt], 4); data_cnt+=4;		/* 現在のSNS-SRAM管理番号 */
	SnsMemory = sns_sram[data_cnt]; data_cnt++;				/* Sensor Memory buffer */
	SnsBackupError = sns_sram[data_cnt]; data_cnt++;			/* バックアップデータエラーフラグ */
	BackupCheckSum = sns_sram[data_cnt]; data_cnt++;			/* バックアップデータのチェックサム */

	/* センサ初期学習状態用構造体 */
	/* センサバックアップ変数構造体へのポインタをセットする */
	bk_l = &SnsLearnStatus;

	/* センサ初期学習状態用構造体 */
	bk_l->SnsAllOk = sns_sram[data_cnt]; data_cnt++;
	bk_l->ShopMode = sns_sram[data_cnt]; data_cnt++;
	bk_l->SpeedOk = sns_sram[data_cnt]; data_cnt++;
	bk_l->GyroOk = sns_sram[data_cnt]; data_cnt++;
	bk_l->GSnsStatus = sns_sram[data_cnt]; data_cnt++;
	bk_l->RevLineOk = sns_sram[data_cnt]; data_cnt++;
	bk_l->YawSetupValue = sns_sram[data_cnt]; data_cnt++;
	data_cnt+=2;	/* アライメント調整分 */

	/* センサバックアップ変数構造体 基本部分へのポインタ */
	/* センサバックアップ変数構造体へのポインタをセットする */
	bk_b = &SensorBackupBasic;

	/* 車速パルスバックアップ変数構造体 基本部分 */
	memcpy(&bk_b->Speed.PeriodLength, &sns_sram[data_cnt], 4); data_cnt+=4;		/* 補正後の１周期の距離(m) */
	memcpy(&bk_b->Speed.PeriodLengthGps, &sns_sram[data_cnt], 4); data_cnt+=4;	/* GPS補正後の１周期の距離(m) */
	memcpy(&bk_b->Speed.PeriodLengthCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* １周期の距離更新カウンタ */
	memcpy(&bk_b->Speed.PrdLenCorrMode, &sns_sram[data_cnt], 2); data_cnt+=2;	/* １周期の距離を補正するモード MANUAL,NOMAL,FAST*/
	bk_b->Speed.PeriodDivision = sns_sram[data_cnt]; data_cnt++;			/* パルスの分周数（1, 2, 4, 8分周が使用可能） */
	bk_b->Speed.PlsLineConnect = sns_sram[data_cnt]; data_cnt++;			/* FALSE:車速パルス未接続,  TRUE:車速パルスあり*/

	data_cnt+=2;	/* アライメント調整分 */

	memcpy(&bk_b->Speed.ManualCorr, &sns_sram[data_cnt], 4); data_cnt+=4;	/* MANUAL距離補正係数 */
	bk_b->Speed.RevLineConnect = sns_sram[data_cnt]; data_cnt++;		/* リバース線が接続されているかを示すフラグ */
	bk_b->Speed.RevLogicSet = sns_sram[data_cnt]; data_cnt++;		/* バース線の論理がセットされているかを示すフラグ */
	bk_b->Speed.ForwardLogic = sns_sram[data_cnt]; data_cnt++;		/* リバースラインの前進論理 */

	data_cnt++;	/* アライメント調整分 */

	/* ジャイロセンサバックアップ変数構造体 基本部分 */
	memcpy(&bk_b->Gyro.Direction, &sns_sram[data_cnt], 4); data_cnt+=4;	/* ジャイロセンサ方位データ */
	memcpy(&bk_b->Gyro.RightGain, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 右ゲイン */
	memcpy(&bk_b->Gyro.LeftGain, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 左ゲイン */
	memcpy(&bk_b->Gyro.CommonGain, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 左右共通ゲイン */
	bk_b->Gyro.RGainCnt = sns_sram[data_cnt]; data_cnt++;			/* 右ゲインの学習度 */
	bk_b->Gyro.LGainCnt = sns_sram[data_cnt]; data_cnt++;			/* 左ゲインの学習度 */
	bk_b->Gyro.RGainCorrCnt = sns_sram[data_cnt]; data_cnt++;		/* 右ゲインの補正回数 */
	bk_b->Gyro.LGainCorrCnt = sns_sram[data_cnt]; data_cnt++;		/* 左ゲインの補正回数 */

	/* Gセンサバックアップ変数構造体 基本部分 */
	bk_b->GSns.GSnsErr = sns_sram[data_cnt]; data_cnt++;		/* 設置方向判定フラグ(1: NG) */
	bk_b->GSns.SettingDir = sns_sram[data_cnt]; data_cnt++;		/* 前後方向判定フラグ(0:不定,1: 反転,2: 正面) */
	bk_b->GSns.DistanceSet = sns_sram[data_cnt]; data_cnt++;	/* 距離使用可能判断フラグ (TRUE: set) */

	data_cnt++;	/* アライメント調整分 */

	memcpy(&bk_b->GSns.Offset, &sns_sram[data_cnt], 4); data_cnt+=4;	/* オフセット値 */
	memcpy(&bk_b->GSns.OffsetCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* オフセットカウンタ */
	bk_b->GSns.SettingSide = sns_sram[data_cnt]; data_cnt++;		/* 回転方向の取り付け向き(右向き:1, 左向き:2) */

	data_cnt++;	/* アライメント調整分 */

	memcpy(&bk_b->GSns.YawTheta, &sns_sram[data_cnt], 4); data_cnt+=4;	/* Yaw方向の取り付け角度 */

	memcpy(&bk_b->GSns.IncGain, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 加速方向のゲイン補正値 */
	memcpy(&bk_b->GSns.DecGain, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 減速方向のゲイン補正値 */
	memcpy(&bk_b->GSns.IncGainCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* 加速方向のゲイン補正カウンタ */
	memcpy(&bk_b->GSns.DecGainCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* 加速方向のゲイン補正カウンタ */
	data_cnt+=4;	/* アライメント調整分 */

	/*** 緯度、経度、高度 ***/
	for (i=0; i<3; i++)  {
		memcpy(&bkCurrPosLla[i], &sns_sram[data_cnt], 8); data_cnt+=8;	/* 緯度、経度、高度 */
	}

	memcpy(&bkWeekNumber, &sns_sram[data_cnt], 2); data_cnt+=2;	/* 週番号 */
	data_cnt+=2;	/* アライメント調整分 */
	memcpy(&bkMsecOfWeek, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 週時間 */

	for (i=0; i<50; i++)  {
		TSnsBackupData.GyroOfsAve[i] = sns_sram[data_cnt]; data_cnt++;
	}

	for (i=0; i<20; i++)  {
		TSnsBackupData.GyroGainAve[i] = sns_sram[data_cnt]; data_cnt++;
	}

	for (i=0; i<50; i++)  {
		TSnsBackupData.GSnsOfsAve[i] = sns_sram[data_cnt]; data_cnt++;
	}

	for (i=0; i<20; i++)  {
		TSnsBackupData.GSnsGainAve[i] = sns_sram[data_cnt]; data_cnt++;
	}

	for (i=0; i<50; i++)  {
		memcpy(&TSnsBackupData.GyroOffset[i], &sns_sram[data_cnt], 2); data_cnt+=2;
	}

	for (i=0; i<20; i++)  {
		memcpy(&TSnsBackupData.GyroGain[i], &sns_sram[data_cnt], 2); data_cnt+=2;
	}

	for (i=0; i<50; i++)  {
		memcpy(&TSnsBackupData.GSnsOffset[i], &sns_sram[data_cnt], 2); data_cnt+=2;
	}

	for (i=0; i<20; i++)  {
		memcpy(&TSnsBackupData.GSnsGain[i], &sns_sram[data_cnt], 2); data_cnt+=2;
	}

	/* センサバックアップ変数構造体 拡張部分へのポインタ */
	/* センサバックアップ変数構造体へのポインタをセットする */
	bk_e = &SensorBackupExtension;

	/* 車速パルスバックアップ変数構造体 拡張部分 */
	memcpy(&bk_e->Speed.MapMatchCorr, &sns_sram[data_cnt], 4); data_cnt+=4;		/* MM距離補正係数 */
	memcpy(&bk_e->Speed.MapMatchCorrCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* MM distance correction counter */
	bk_e->Speed.PulseErr = sns_sram[data_cnt]; data_cnt++;				/* pulse error backup */
	bk_e->Speed.PulseMode = sns_sram[data_cnt]; data_cnt++;				/* speed pulse filter mode */
	bk_e->Speed.PeriodLengthErr = sns_sram[data_cnt]; data_cnt++;			/* PeriodLength が異常値で使用できないことを示す。 */
	bk_e->Speed.LowSpeedNg = sns_sram[data_cnt]; data_cnt++;			/* 低速車速パルス未発生車両判断フラグ */
	memcpy(&bk_e->Speed.MinimumSpeedCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* 最低出力速度学習回数 */
	memcpy(&bk_e->Speed.MinimumSpeed, &sns_sram[data_cnt], 4); data_cnt+=4;		/* 最低出力速度学習値 [m/sec] */

	memcpy(&bk_e->Speed.LastPrdLen, &sns_sram[data_cnt], 4); data_cnt+=4;		/* タイヤ径検出 前回補正前距離学習値 */

	memcpy(&bk_e->Speed.LastPrdLenCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* タイヤ径検出 距離学習回数カウンタ */
	memcpy(&bk_e->Speed.TyreErrCnt, &sns_sram[data_cnt], 2); data_cnt+=2;		/* 検出判断用カウンタ */
	memcpy(&bk_e->Speed.PhantomPrdLen, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 裏学習値[m] */

	bk_e->Speed.PulseSignalError = sns_sram[data_cnt]; data_cnt++;			/* 車速パルス異常 */
	bk_e->Speed.PastDivNum = sns_sram[data_cnt]; data_cnt++;			/* 過去分周比(JIS基準、分周比異常車両用) */

	data_cnt+=2;	/* アライメント調整分 */

	/* ジャイロセンサバックアップ変数構造体 拡張部分 */
	/* エクセレント学習セット */
	bk_e->Gyro.ExcellentQuality = sns_sram[data_cnt]; data_cnt++;			/* エクセレント補正関数のクオリティ 0:関数なし */

	data_cnt++;	/* アライメント調整分 */

	memcpy(&bk_e->Gyro.ExcellentWeek, &sns_sram[data_cnt], 2); data_cnt+=2;		/* エクセレントを学習した週 */
	memcpy(&bk_e->Gyro.ExcellentLimit, &sns_sram[data_cnt], 4); data_cnt+=4;	/* エクセレント補正の終了時間 */

	for (j=0; j<=3; j++)  {
		memcpy(&bk_e->Gyro.ExcellentFunction[j], &sns_sram[data_cnt], 4); data_cnt+=4;	/* エクセレント補正の回帰曲線の係数 3次関数に対応 */
	}

	memcpy(&bk_e->Gyro.ExcellentAve, &sns_sram[data_cnt], 4); data_cnt+=4;	/* エクセレント学習をしたときのオフセットの平均 */

	memcpy(&bk_e->Gyro.Offset, &sns_sram[data_cnt], 4); data_cnt+=4;	/* オフセット値 */
	memcpy(&bk_e->Gyro.DirErr, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 方位誤差 */
	memcpy(&bk_e->Gyro.DirErrRelief, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 方位誤差(リリーフ動作用) */
	memcpy(&bk_e->Gyro.SigmaLevel, &sns_sram[data_cnt], 4); data_cnt+=4;	/* Gyro 停止判定のシグマの閾値 */

	memcpy(&bk_e->Gyro.GainMovingAve[0], &sns_sram[data_cnt], 4); data_cnt+=4;	/* 最新ジャイロゲイン生値平均値 */
	memcpy(&bk_e->Gyro.GainMovingAve[1], &sns_sram[data_cnt], 4); data_cnt+=4;	/* 最新ジャイロゲイン生値平均値 */
	bk_e->Gyro.SettingChangeCnt[0] = sns_sram[data_cnt]; data_cnt++;		/* 設置角度変更検出カウンター */
	bk_e->Gyro.SettingChangeCnt[1] = sns_sram[data_cnt]; data_cnt++;		/* 設置角度変更検出カウンター */
	bk_e->Gyro.SensitivityErrCnt = sns_sram[data_cnt]; data_cnt++;			/* 感度異常検出用カウンタ */

	bk_e->Gyro.GyroDeviceError = sns_sram[data_cnt]; data_cnt++;			/* Gyroセンサ異常 */
	bk_e->Gyro.AdcDeviceError  = sns_sram[data_cnt]; data_cnt++;			/* A/Dコンバータ異常 */

	bk_e->Gyro.TsnsDeviceError  = sns_sram[data_cnt]; data_cnt++;			/* 温度センサ異常 */

	data_cnt+=2;	/* アライメント調整分 */

	/* Gセンサバックアップ変数構造体 拡張部分 */
	memcpy(&bk_e->GSns.SigmaLevel, &sns_sram[data_cnt], 4); data_cnt+=4;	/* G-Sensor 停止判定のシグマの閾値 */
	memcpy(&bk_e->GSns.SlopeAngle, &sns_sram[data_cnt], 4); data_cnt+=4;	/* 傾斜角 ( down:[-], up:[+] ) */
	memcpy(&bk_e->GSns.YawThetaCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* Yaw方向の取り付け角度学習回数 */
	bk_e->GSns.SettingSideCnt = sns_sram[data_cnt]; data_cnt++; 		/* 回転方向の取り付け向き学習カウンタ */

	data_cnt++;	/* アライメント調整分 */

	memcpy(&bk_e->GSns.CommonGain, &sns_sram[data_cnt], 4); data_cnt+=4;		/* 加減速共通ゲイン */
	memcpy(&bk_e->GSns.InstallPitchAngle, &sns_sram[data_cnt], 4); data_cnt+=4; 	/* ピッチ方向取り付け角度 */
	bk_e->GSns.Reserve = sns_sram[data_cnt]; data_cnt++; 				/* Reserve */
	data_cnt+=3;	/* アライメント調整分 */
	/* センサ初期学習状態用構造体へのポインタ */
	SnsBkL = &SnsLearnStatus;

	/* センサバックアップ変数構造体 基本部分へのポインタ */
	SnsBkB = &SensorBackupBasic;

	/* センサバックアップ変数構造体 拡張部分へのポインタ */
	SnsBkE = &SensorBackupExtension;

	memcpy(&SnsBkM.PulseCounter, &sns_sram[data_cnt], 4); data_cnt+=4;	/* pulse counter */
	data_cnt+=4;	/* アライメント調整分 */

	memcpy(&SnsBkM.Odometer, &sns_sram[data_cnt], 8); data_cnt+=8;	/* odometer */
	memcpy(&SnsBkM.ScheOdometer, &sns_sram[data_cnt], 8); data_cnt+=8;	/* odometer */
	memcpy(&SnsBkM.DeltaLatSum, &sns_sram[data_cnt], 4); data_cnt+=4;	/* backup for Device Driver Switch */
	memcpy(&SnsBkM.DeltaLonSum, &sns_sram[data_cnt], 4); data_cnt+=4;	/* backup for Device Driver Switch */
	memcpy(&SnsBkM.DeltaDSum, &sns_sram[data_cnt], 4); data_cnt+=4;	/* backup for Device Driver Switch */
	SnsBkM.DeltaSumOn = sns_sram[data_cnt]; data_cnt++;			/* backup for Device Driver Switch */

	data_cnt+=3;	/* アライメント調整分 */

	memcpy(&SnsBkM.CanBusEndOfLineSetting , &sns_sram[data_cnt], 4); data_cnt+=4;	/* CAN-BUSからのEOL(end-of-line)設定情報 バックアップ変数 */

	data_cnt+=4;	/* アライメント調整分 */

	/* 傾斜旋回補正誤差見積もり */
	memcpy(&SnsBkIncEstErr.DegreeOfSnsDirErrorPosi, &sns_sram[data_cnt], 4); data_cnt+=4;	/* センサ方位誤差積算[DEG] */
	memcpy(&SnsBkIncEstErr.DegreeOfSnsDirErrorPosiAbs, &sns_sram[data_cnt], 4); data_cnt+=4;	/* センサ方位誤差積算(絶対値)[DEG] */
	memcpy(&SnsBkIncEstErr.DegreeOfSnsDirErrorNega, &sns_sram[data_cnt], 4); data_cnt+=4;	/* センサ方位誤差積算[DEG] */
	memcpy(&SnsBkIncEstErr.DegreeOfSnsDirErrorNegaAbs, &sns_sram[data_cnt], 4); data_cnt+=4;	/* センサ方位誤差積算(絶対値)[DEG] */
	memcpy(&SnsBkIncEstErr.SetupAngleDeg, &sns_sram[data_cnt], 4); data_cnt+=4;			/* 推測取付け角度の絶対値 単位[度] */

	/* CanBus車速パルス異常検出用構造体 */
	CanPulseErr.DivNumErrLevel = sns_sram[data_cnt]; data_cnt++;    /* 分周比異常レベル(JIS基準) */
	CanPulseErr.NonlinearLevel = sns_sram[data_cnt]; data_cnt++;    /* 非線形性レベル */
	CanPulseErr.StepwiseLevel = sns_sram[data_cnt]; data_cnt++;	/* 階段状レベル */
	CanPulseErr.AlternusLevel = sns_sram[data_cnt]; data_cnt++;	/* 交互変化レベル */
	CanPulseErr.Reserve1 = sns_sram[data_cnt]; data_cnt++;		/* Reserve */
	CanPulseErr.Reserve2 = sns_sram[data_cnt]; data_cnt++;		/* Reserve */
	CanPulseErr.Reserve3 = sns_sram[data_cnt]; data_cnt++;		/* Reserve */
	CanPulseErr.Reserve4 = sns_sram[data_cnt]; data_cnt++;		/* Reserve */

	/* 速度に対する距離学習値保存用構造体 */
	for (i=0; i<7; i++)  {
		memcpy(&PeriodLengthEx[i].PrdLength, &sns_sram[data_cnt], 4); data_cnt+=4;		/* 距離学習値格納バッファ */
		memcpy(&PeriodLengthEx[i].SpeedAve, &sns_sram[data_cnt], 4); data_cnt+=4;		/* 距離学習時の平均速度 */
		memcpy(&PeriodLengthEx[i].PrdLengthCnt, &sns_sram[data_cnt], 2); data_cnt+=2;	/* 各速度の距離学習回数 */
		data_cnt+=2;	/* アライメント調整分 */
	}

	/* 距離学習値近似曲線用構造体 */
	for (i=0; i<4; i++)  {
		memcpy(&ApxPeriodLengthData.PrdLengthCorr[i], &sns_sram[data_cnt], 4); data_cnt+=4;	/* 距離学習値近似曲線係数格納バッファ */
	}
	ApxPeriodLengthData.PrdLengthProcess = sns_sram[data_cnt]; data_cnt++;	/* 距離学習値近似曲線算出ステータス */
	ApxPeriodLengthData.UseBufCnt = sns_sram[data_cnt]; data_cnt++;	        /* 距離学習値近似曲線算出使用バッファ数 */
	ApxPeriodLengthData.PrdLengthOk = sns_sram[data_cnt]; data_cnt++;	/* 距離学習値近似曲線使用可能フラグ */
	data_cnt++;	/* アライメント調整分 */

	sns_bat_sign = sns_sram[data_cnt]; data_cnt++;		/* signature for battery RAM */

#endif  /* COMP_OPT_FOR_SNS_SIMULATOR */
}

/******************************************************************************
  【処理】センサのSRAMエリアのチェックサムを計算する
  【戻値】計算したチェックサム
  【解説】センサのSRAMエリアのチェックサムを計算する。
******************************************************************************/
BYTE NPSnsHandlerBase::CalcSnsBackupCheckSum(VOID)
{
    BYTE  *p = NULL;
    BYTE  cs = 0x00;

    /*** チェックサムの計算 ***/
    /* バックアップデータの途中にチェックサムがあるため */
    /* 全体のチェックサムを計算すると0になるはず */ 
    for (p = (BYTE *)&SNS_SIGNWORD; p <= (BYTE *)&sns_bat_sign; p++)  {
	cs ^= *p;
    }
    /* 計算した結果を返す */
    return (cs);
}

/******************************************************************************
  【処理】センサのSRAMエリアのチェックサムを返す(公開関数)
  【戻値】計算したチェックサム
  【解説】センサのバックアップセクションのチェックサムを計算する
	  関数(CalcSnsBackupCheckSum())を呼び出し値を返す。
******************************************************************************/
BYTE NPSnsHandlerBase::CheckSnsBackupSum(VOID)
{
    return  (CalcSnsBackupCheckSum());
}

/******************************************************************************
  【処理】SNS_SIGNWORDを元にデータのチェックを行う
  【戻値】TRUE  : 正常
	  FALSE : 異常
  【解説】SNS_SIGNWORDを元にデータのチェックを行う
******************************************************************************/
BYTE NPSnsHandlerBase::VerifySensorBackupData(VOID)
{

    /* **********************************************************
       バージョンアップ処理は、バージョン順に列挙し処理を行うこと
       また、最新版は最後にすること！！
    ********************************************************** */
   
    /* 最新版として有効なバックアップデータの場合 */
    if ((SNS_SIGNWORD == SNS_SIGNATURE)
     && (CheckSnsBackupSum() == 0x00)
     && (sns_bat_sign == 'N')
     && (SnsSRamNow == (DWORD)SNS_SRAM))  { /* 最新バージョンの場合 */

	/* チェックＯＫ、正常値(TRUE)を返す */
	return (TRUE);
    }

    return (FALSE);
}

/******************************************************************************
【処理】センサバックアップデータSRAM読み込み処理関数(Aセクション用)
【戻値  読み込み状態 NP_TRUE:OK NP_FALSE:NG
【解説】SRAMからセンサのバックアップの読み込みを行う(Aセクション用)
******************************************************************************/
NP_BOOL
NPSnsHandlerBase::ReadToASectionSnsBkSRAM( VOID )
{
#ifdef COMP_OPT_FOR_MACHINE

	// SRAMへの読み込み処理
	// Aセクションからのセンサ学習値の読み込み 
	CopySnsBkSRAM(NP_SNS_A_SECTION);

	/* バックアップセクションの有効性のチェックがNG */
	if (VerifySensorBackupData() == FALSE)  {
		/* センサのバックアップ変数全てを初期化する */
		InitSensorBackupAll();
		/* センサバックアップデータ復帰出来ず */
		MakeErrorInfo2(121, 0, __LINE__, VERSION);
		// 読み込みNG
		return (NP_FALSE);
	}

	/* 店頭展示モードになっているかチェックする */
	if (SnsLearnStatus.ShopMode == TRUE)  {
		/* センサのバックアップ変数全てを初期化する */
		InitSensorBackupAll();
		/* センサバックアップデータ復帰出来ず */
		MakeErrorInfo2(121, 1, __LINE__, VERSION);
		// 読み込みNG
		return (NP_FALSE);
	}

	// 読み込みOK
	return (NP_TRUE);

#endif	// COMP_OPT_FOR_MACHINE

	// 読み込みNG
	return (NP_FALSE);

}

/******************************************************************************
【処理】センサバックアップデータSRAM読み込み処理関数(Bセクション用)
【戻値  読み込み状態 NP_TRUE:OK NP_FALSE:NG
【解説】SRAMからセンサのバックアップの読み込みを行う(Bセクション用)
******************************************************************************/
NP_BOOL
NPSnsHandlerBase::ReadToBSectionSnsBkSRAM( VOID )
{
#ifdef COMP_OPT_FOR_MACHINE

	// SRAMへの読み込み処理
	// Bセクションからのセンサ学習値の読み込み 
	CopySnsBkSRAM(NP_SNS_B_SECTION);

	/* バックアップセクションの有効性のチェックがNG */
	if (VerifySensorBackupData() == FALSE)  {
		/* センサのバックアップ変数全てを初期化する */
		InitSensorBackupAll();
		/* センサバックアップデータ復帰出来ず */
		MakeErrorInfo2(122, 0, __LINE__, VERSION);
		// 読み込みNG
		return (NP_FALSE);
	}

	/* 店頭展示モードになっているかチェックする */
	if (SnsLearnStatus.ShopMode == TRUE)  {
		/* センサのバックアップ変数全てを初期化する */
		InitSensorBackupAll();
		/* センサバックアップデータ復帰出来ず */
		MakeErrorInfo2(122, 1, __LINE__, VERSION);
		// 読み込みNG
		return (NP_FALSE);
	}

	// 読み込みOK
	return (NP_TRUE);

#endif	// COMP_OPT_FOR_MACHINE

	// 読み込みNG
	return (NP_FALSE);
}

/******************************************************************************
【処理】モデル別の動作状態のセット
【戻値】なし
【解説】モデル別の動作状態構造体に値をセットする sns_model.hを参照
******************************************************************************/
VOID NPSnsHandlerBase::SetSnsModelInfo(VOID)
{
	/* モデル別の動作状態構造体に値をセットする */
	/* TRUE GSNSあり FALSE GSNSなし */
	SnsModelInfo.GSnsBuiltin = GSNS_BUILTIN;
	/* TRUE TSNSあり FALSE TSNSなし */
	SnsModelInfo.TSnsBuiltin = TSNS_BUILTIN;
	/* TRUE CAN-BUSからのウィンカー情報あり FALSE CAN-BUSからのウィンカー情報なし */
	SnsModelInfo.CanWinkerInfo = CAN_WINKER_INFO;
	/* TRUE ふらつき検知あり FALSE ふらつき検知なし */
	SnsModelInfo.WeavingDetect = WEAVING_DETECT;
	/* 仕向け */
	SnsModelInfo.Destination = PRODUCT_DEST;
	/* 車両実装時のセンサ検出軸の傾き角度[deg] */
	SnsModelInfo.InstallGrade = static_cast< FLOAT >((FLOAT)RAD2DEG * acosf(1.0F/INSTALL_GRADE));
	/* 未学習でも走行を開始するか？ */
	SnsModelInfo.DefaultMove = DEFAULT_MOVE;
	/* 前進論理 */
	SnsModelInfo.ForwardLogic = FORWARD_LOGIC;
	/* A2Dコンバータのバージョン */
	SnsModelInfo.AdcChip = ADC_TYPE;
	/* ジャイロのバージョン */
	SnsModelInfo.GyroDevice = GYRO_TYPE;
	/* G-Sensorのバージョン */
	SnsModelInfo.GSnsDevice = GSNS_TYPE;
	/* 温度センサのバージョン */
	SnsModelInfo.TSnsDevice = TSNS_TYPE;
	/* タイヤ外径変化検出の補正方法 */
	SnsModelInfo.TyreChange = TYRE_CHANGE;
	/* 取付け角度変更検出の補正方法 */
	SnsModelInfo.SettingChange = SETTING_CHANGE;
	/* Gセンサの取付け向き */
	SnsModelInfo.GSnsInstall = GSNS_INSTALL;
	/* 車速パルス信号情報源 */
	SnsModelInfo.VssSource = VSS_SOURCE;
	/* リバース信号情報源 */
	SnsModelInfo.RevSource = REV_SOURCE;
	/* パルスフィルタがPSW OUTに接続されているか？ TRUE 接続 FALSE 非接続 */
	SnsModelInfo.PulseFilter = PULSE_FILTER;

}

/******************************************************************************
【処理】GPSデバッグ情報データセット
【戻値】なし
【解説】新しいエラー番号の場合はバッファポインタをシフトし、バッファが一杯の
場合は一番古いものの上に上書きする
同じエラー番号が続いた場合は回数のみインクリメントする。
******************************************************************************/
VOID NPSnsHandlerBase::MakeErrorInfo2(
									  BYTE	e_num,		/* エラー番号 */
									  DWORD	value,		/* 任意の値 */
									  DWORD	lines,		/* エラーが発生した行番号 */
									  const CHAR	*id		/* VERSIONで定義したRCS ID文字列へのポインタ */
									  )
{

	NPGpsErrorInfoStr	*ep = NULL;		/* エラー履歴構造体へのポインタ */
	sGpsSnsMail		*gps_sns_ptr = NULL;	/* メールデータへのポインタ */
	INT    i = 0;				/* ローカルカウンタ */

	/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)

	// エラー履歴へのポインタをセット
	ep = &bkFatalErrorArray[0];

	// 一番初めのエラー履歴追加時
	if (ep->ErrorNumber == 0) {
		/* エラーを追加 */
		ep->ErrorCount = 1;
		ep->ErrorNumber = e_num;
	}
	/* 追加するエラー番号が違っていたら */
	else if (ep->ErrorNumber != e_num)  {
		// 共有メモリのエラー履歴をソートする
		// 10個以上のエラー履歴は、消去される
		for (i=9; i>0; i--) {
			memcpy(&bkFatalErrorArray[i], &bkFatalErrorArray[i-1], sizeof( NPGpsErrorInfoStr ));
		}
		// 最新のエラー履歴を0フィルする
		memset(&bkFatalErrorArray[0], 0, sizeof( NPGpsErrorInfoStr ));

		/* エラーを追加 */
		ep->ErrorCount = 1;
		ep->ErrorNumber = e_num;
		/* エラー履歴追加カウンタをインクリメント */
		ExecuteMakeErrorInfo2++;
	}
	/* エラー番号が同一でカウント値が255未満なら */
	else if (ep->ErrorCount < 255)  {
		/* カウント値をインクリメント */
		ep->ErrorCount++;
	}

#else   /* !COMP_OPT_FOR_SNS_SIMULATOR */

	//com.pset
	return;

	// 排他オブジェクト取得
	//m_cData_SyncObject.Create(NP_GPS_DATA_SYNC);
	//m_cData_SyncObject.SynchronizeStart( );

	// エラー履歴へのポインタをセット
	ep = &m_psDataStatus->sGpsError[0];
	// メールデータのポインタをセット
	gps_sns_ptr = &GpsSnsMailBuff;


	/* 追加するエラー番号が違っていたら */
	if (ep->ErrorNumber != e_num)  {
		// 共有メモリのエラー履歴をソートする
		// 10個以上のエラー履歴は、消去される
		for (i=9; i>0; i--) {
			memcpy(&m_psDataStatus->sGpsError[i], &m_psDataStatus->sGpsError[i-1], sizeof( NPGpsErrorInfoStr ));
		}

		/* エラーを追加 */
		ep->ErrorCount = 1;
		ep->ErrorNumber = e_num;
	}
	/* エラー番号が同一でカウント値が255未満なら */
	else if (ep->ErrorCount < 255)  {
		/* カウント値をインクリメント */
		ep->ErrorCount++;
	}

#endif
	/* GPS週記録 */
	ep->WeekNumber = LatestWeekNumber;

	// Time Of Week を記録
	ep->TimeOfWeek = LatestMsecOfWeek;
	ep->DayOfWeek = static_cast< BYTE >(ep->TimeOfWeek / MAX_DAY_SEC);

	/* latitude [RAD] -> [1/256 SEC] */
	ep->Latitude = static_cast< LONG >(bkCurrPosLla[0] * RAD2SEC * 256.0);

	/* longitude [RAD] -> [1/256 SEC] */
	ep->Longitude = static_cast< LONG >(bkCurrPosLla[1] * RAD2SEC * 256.0);

	/* altitude [m] */
	ep->Altitude = static_cast< SHORT >(bkCurrPosLla[2]);

	/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)
#else   /* !COMP_OPT_FOR_SNS_SIMULATOR */
	/* velocity [m/s] -> [km/h] */
	ep->Velocity = static_cast< WORD >(gps_sns_ptr->Velocity * 36.0F);

#endif	/* COMP_OPT_FOR_SNS_SIMULATOR */
	/* RTCバッファの値を取得 */
	GetCurTime(&ep->RtcTime[0]);

	ep->Value = value;
	ep->Lines = static_cast< WORD >(lines);

	/* id で渡される文字列が82文字以下の場合は残りはNULLで埋められる */
	strncpy(ep->Id, id, sizeof(ep->Id));

	/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)
#else   /* !COMP_OPT_FOR_SNS_SIMULATOR */
	// エラー履歴情報受信済みフラグ
	m_psDataStatus->bGpsReceiveErrorInf = NP_TRUE;

	/* エラー履歴復帰後 */
	if (ErrorInfoCondition == TRUE)  {
		// SRAMへの保存
		WriteToErrInfoSRAM();
	}
	/* エラー履歴復帰前 */
	else  {
		// センサ学習データ復帰前のため、エラー情報を退避する
		StoreErrorInfo(e_num, value, lines, id);
	}

	// エラー履歴更新フラグを立てる
	MakeErrorInfoFlag = NP_TRUE;

	//com.pset
	//// 排他オブジェクトの終了
	//m_cData_SyncObject.SynchronizeEnd();
	//m_cData_SyncObject.Destroy();

#endif	/* COMP_OPT_FOR_SNS_SIMULATOR */
}

/******************************************************************************
【処理】メールデータバックアップ変数構造体の初期化
【戻値】なし
【解説】メールデータバックアップ変数構造体の初期化
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorBackupMailData(VOID)
{
	SnsBkM.PulseCounter	= 0;		/* パルスカウンタ */
	SnsBkM.Odometer	= 0.0;		/* オドメータ */
	SnsBkM.ScheOdometer	= 0.0;		/* スケジューラ用オドメータ */
	SnsBkM.DeltaLatSum	= 0.0F;		/* メール非送信時の移動量保存 緯度方向 */
	SnsBkM.DeltaLonSum	= 0.0F;		/*   経度方向 */
	SnsBkM.DeltaDSum	= 0.0F;		/*   走行距離 */
	SnsBkM.DeltaSumOn	= FALSE;	/*   移動量保存済みフラグ */

	/*** CAN-BUSからのEOL(end-of-line)設定情報 バックアップ変数の初期化 ***/
	InitCanBusBkPrms();
}

/******************************************************************************
  【処理】CAN-BUS関連バックアップデータ初期化関数
  【戻値】なし
  【解説】CAN-BUS関連バックアップデータの初期化処理を行う。
******************************************************************************/
VOID NPSnsHandlerBase::InitCanBusBkPrms(VOID)
{
    /*** CAN-BUSからのEOL(end-of-line)設定情報 バックアップ変数の初期化 ***/
    SnsBkM.CanBusEndOfLineSetting = 0;
    /* EOL VSS 設定値の初期化 */
    SnsBkM.CanBusEndOfLineSetting ^= CAN_EOL_HARDWIRE_VSS;
    /* EOL リバース 設定値の初期化 */
    SnsBkM.CanBusEndOfLineSetting ^= CAN_EOL_MSCAN_HARDWIRE_REV;
}


/******************************************************************************
【処理】センサバックアップメモリクリア
【戻値】なし
【解説】センサ学習に使用するバックアップ変数のみを初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorBackup(VOID)
{
	/* センサバックアップ変数構造体 基本部分へのポインタ */
	sSensorBackupBasic		*bk_b = NULL;
	/* センサバックアップ変数構造体 拡張部分へのポインタ */
	sSensorBackupExtension	*bk_e = NULL;
	/* センサバックアップ変数構造体 初期学習状態部分へのポインタ */
	sSnsLearnStatus		*bk_l = NULL;

	/* センサバックアップ変数構造体 基本部分へのポインタをセットする */
	bk_b = &SensorBackupBasic;
	/* センサのバックアップ変数のうち、基本となる部分を初期化する */
	InitSensorBackupBasic(bk_b);

	/* センサバックアップ変数構造体 拡張部分へのポインタをセットする */
	bk_e = &SensorBackupExtension;
	/* センサのバックアップ変数のうち、拡張部分を初期化する */
	InitSensorBackupExtension(bk_e);

	/* センサバックアップ変数構造体 初期学習状態部分へのポインタをセットする */
	bk_l = &SnsLearnStatus;
	/* センサのバックアップ変数のうち、初期学習状態部分へのポインタ */
	InitSnsLearnStatus(bk_l);
}

/******************************************************************************
【処理】センサバックアップ変数初期化
【戻値】なし
【解説】センサのバックアップ変数のうち、基本となる部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	)
{
	/* 車速パルス */
	InitSpeedBackupBasic(bk_b);

	/* ジャイロセンサ */
	InitGyroBackupBasic(bk_b);

	/* Gセンサ */
	InitGSnsBackupBasic(bk_b);
}

/******************************************************************************
【処理】センサバックアップデータをすべて初期化する
【戻値】なし
【解説】センサのバックアップデータをすべて初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorBackupAll(VOID)
{
	/* モデル別の動作状態をセットする */
	SetSnsModelInfo();

	/* メールデータバックアップ変数を初期化する */
	InitSensorBackupMailData();

	/* センサバックアップデータ領域を初期化する */
	InitSensorBackup();

	/* センサ内 GPS関連バックアップデータの初期化 */
	InitGpsBackup();

	/* 温度センサ バックアップ変数初期化 */
	InitTSnsBackupData();

	/* 傾斜旋回補正の見積もり誤差関連の全データを初期化する */
	InitIncEstErrorParameters();

	/* CanBus車速パルス異常検出用関連の全データを初期化する */
	InitCanPulseErr();

	/* 速度に対する距離学習値関連の全データを初期化する */
	InitPeriodLengthEx();

	/* 距離学習値近似曲線関連の全データを初期化する */
	InitApxPeriodLengthData();

	/* A用メモリ使用 */
	SnsMemory = GYRO_MEM_A;

	/* センサ初期学習状態用構造体へのポインタをセット */
	SnsBkL = &SnsLearnStatus;
	/* センサバックアップ変数構造体 基本部分へのポインタをセット */
	SnsBkB = &SensorBackupBasic;
	/* センサバックアップ変数構造体 拡張部分へのポインタをセット */
	SnsBkE = &SensorBackupExtension;

	/* SNS_SIGNWORD, sns_bat_signにチェックデータを書き込む */
	WriteSensorBackupSignature();

	/* センサのバックアップのチェックサムを計算 */
	AddSnsBackupCheckSum(NP_FALSE);

}

/******************************************************************************
  【処理】SNS_SIGNWORD, sns_bat_signにチェックデータを書き込む
  【戻値】なし
  【解説】SNS_SIGNWORD, sns_bat_signにチェックデータを書き込む
******************************************************************************/
VOID NPSnsHandlerBase::WriteSensorBackupSignature(VOID)
{

    /* set Sensor SIGNATURE */
    SNS_SIGNWORD = static_cast< WORD >(SNS_SIGNATURE);

    /* set sns_bat_sign as 'N' */
    sns_bat_sign = 'N';

    /* 管理番号をセット */
    SnsSRamNow = static_cast< DWORD >(SNS_SRAM);
}

/******************************************************************************
  【処理】センサのSRAMエリアにチェックサムをセットする
  【戻値】なし
  【解説】センサのバックアップセクションのチェックサムを計算し、
	  バックアップ変数BackupCheckSumに保存する。
******************************************************************************/
VOID NPSnsHandlerBase::AddSnsBackupCheckSum(
    NP_BOOL req_to_send	/* バックアップの送信要求 */
    )
{
    /* バックアップデータの途中にチェックサム(BackupCheckSum)があるため、*/
    /* チェックサムを取る前に0で初期化しておく */
    BackupCheckSum = 0;
    /* 計算したチェックサムをセーブする */
    BackupCheckSum = CalcSnsBackupCheckSum();

#ifdef COMP_OPT_FOR_MACHINE

    // SRAMへセンサ学習データを書き込む
    WriteToSnsBkSRAM();

#endif	// COMP_OPT_FOR_MACHINE

}

/******************************************************************************
【処理】センサバックアップデータSRAM保存処理関数
【戻値】なし
【解説】センサのバックアップをSRAMへ保存する
******************************************************************************/
VOID
NPSnsHandlerBase::WriteToSnsBkSRAM( VOID )
{
#ifdef COMP_OPT_FOR_MACHINE

	//com.pset
	//NPSramData  cSram;			// SRAMアクセスクラス
	PBYTE	start_add = NULL;	// 読み出し開始位置

	// センサバックアップデータが復帰していない場合
	if (SnsMemCondition == NP_SNS_MEM_CONDITION_NG)  {
		// センサ学習データの読み込み処理が終了するまで
		// メモリへの書き込み処理は行わせないようにする
		return;
	}
	// 読み込み開始アドレスの設定
	start_add = (PBYTE)&SNS_SIGNWORD;

	//// SRAMへの書き込み処理
	//// Aセクションへセンサ学習値の書き込み 
	//if ((cSram.Write(NP_SRAM_AREA_SNS, SNS_LEARN_BK_DATA_A , start_add, static_cast< DWORD >( AdjSnsMemSize ))) == NP_FALSE)  {
	//	// 失敗時
	//	// システムとして動作できないので、エラーログを残す
	//	NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//}

	//// Bセクションへセンサ学習値の書き込み 
	//if ((cSram.Write(NP_SRAM_AREA_SNS, SNS_LEARN_BK_DATA_B , start_add, static_cast< DWORD >( AdjSnsMemSize ))) == NP_FALSE)  {
	//	// 失敗
	//	// システムとして動作できないので、エラーログを残す
	//	NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//}

#endif	// COMP_OPT_FOR_MACHINE
}


/******************************************************************************
【処理】車速パルス バックアップ変数初期化
【戻値】なし
【解説】車速パルス関連のバックアップ変数の基本となる部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSpeedBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	)
{
	/* 車速パルス */
	bk_b->Speed.PeriodLength	= 0.0F;		/* 補正後の距離学習値[m] */
	bk_b->Speed.PeriodLengthGps	= 0.0F;		/* GPS補正後の距離学習値[m] */
	bk_b->Speed.PeriodLengthCnt	= 0;		/* 距離学習回数 */
	bk_b->Speed.PrdLenCorrMode	= NORMAL_CORRECT; /* 自動補正/手動補正モード */
	bk_b->Speed.PeriodDivision = 1;		/* 分周比設定値(数値=分周比) */
	bk_b->Speed.PlsLineConnect	= TRUE;		/* パルス線接続状況(接続/非接続) */
	bk_b->Speed.ManualCorr	= 1.0F;		/* 手動距離補正係数 */

	/*** 前進論理判断フラグ ***/
	/* モデル固有の前進論理が不明の場合 */
	if (SnsModelInfo.ForwardLogic == REV_UNKNOWN)  {
		/* リバース線接続状態不明 (接続/非接続) */
		bk_b->Speed.RevLineConnect = FALSE;
		/* 後退論理の確立判断フラグ */
		bk_b->Speed.RevLogicSet =  FALSE;
		/* リバース端子の電圧が0[V]の時を前進と判断する */
		bk_b->Speed.ForwardLogic = REV_SNS_L;
	}
	/* モデル固有の前進論理が機知の場合 */
	else  {
		/* リバース線接続済み */
		bk_b->Speed.RevLineConnect = TRUE;
		/* 後退論理は決定済み */
		bk_b->Speed.RevLogicSet = TRUE;
		/* モデル固有の前進論理をセットする */
		bk_b->Speed.ForwardLogic = SnsModelInfo.ForwardLogic;
	}
}

/******************************************************************************
【処理】ジャイロ バックアップ変数初期化
【戻値】なし
【解説】ジャイロ関連のバックアップ変数の基本となる部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitGyroBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	)
{
	/* TOYOTAモデル固有の設定 */
	if (SnsModelInfo.Destination == TOYOTA)  {
		bk_b->Gyro.Direction = 90.0F;	/* 初期方位:東 */
	}
	/* HONDAモデル固有の設定 */
	else if (SnsModelInfo.Destination == HONDA)  {
		bk_b->Gyro.Direction = 90.0F;	/* 初期方位:東 */
	}
	/* FORDモデル固有の設定 */
	else if (SnsModelInfo.Destination == FORD)  {
		bk_b->Gyro.Direction = 90.0F;	/* 初期方位:東 */
	}
	else  {
		bk_b->Gyro.Direction = 0.0F;	/* Gyro方位 */
	}

	bk_b->Gyro.RightGain = GAIN_DEF;	/* GYRO right gain */
	bk_b->Gyro.LeftGain  = GAIN_DEF;	/* GYRO left gain */
	bk_b->Gyro.CommonGain = GAIN_DEF;	/* 左右共通ゲイン */
	bk_b->Gyro.RGainCnt     = 0;	/* GYRO right gain counter */
	bk_b->Gyro.LGainCnt     = 0;	/* GYRO left gain counter */
	bk_b->Gyro.RGainCorrCnt = 0;	/* GYRO right gain correction counter */
	bk_b->Gyro.LGainCorrCnt = 0;	/* GYRO left gain correction counter */
}

/******************************************************************************
【処理】Gセンサ バックアップ変数初期化
【戻値】なし
【解説】Gセンサ関連のバックアップ変数の基本となる部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitGSnsBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	)
{

	FLOAT gsns_ofs_sin = 0.0F;	/* Gセンサが検出する重力加速度のsin成分値[LSB] */

	/* Gセンサ */
	bk_b->GSns.GSnsErr     = FALSE;		/* 設置方向判定フラグ(1: NG) */

	if (SnsModelInfo.Destination == HONDA)  {
		bk_b->GSns.SettingDir  = FRONT_SET;	/* 前後方向判定フラグ : 正面 */ 

		/* 車両実装時のGセンサ傾き角度を考慮したオフセット値を設定する */
		gsns_ofs_sin = static_cast< FLOAT >(G_A * sinf((FLOAT)DEG2RAD * SnsModelInfo.InstallGrade) / G_GAIN);
		bk_b->GSns.Offset      = GSNS_OFS_DEF - gsns_ofs_sin;

		bk_b->GSns.OffsetCnt   = 1;		/* オフセット値学習カウンタ */
	}
	else  {
		bk_b->GSns.SettingDir  = UNKNOWN_SET;	/* 前後方向判定フラグ : 不定 */ 
		bk_b->GSns.Offset      = GSNS_OFS_DEF;	/* オフセット値 */
		bk_b->GSns.OffsetCnt   = 0;		/* オフセット値学習カウンタ */
	}

	bk_b->GSns.DistanceSet = FALSE;		/* 距離使用可能判断フラグ (1: set) */
	bk_b->GSns.SettingSide = FALSE;		/* 回転方向の取り付け向き*/
	/* 0:不定, 1:右向き, 2:左向き */
	bk_b->GSns.YawTheta    = 0.0F;		/* Yaw方向の取り付け角度 */
	bk_b->GSns.IncGain     = GSNS_GAIN_DEF;	/* 加速方向のゲイン補正値 */
	bk_b->GSns.DecGain     = GSNS_GAIN_DEF;	/* 減速方向のゲイン補正値 */
	bk_b->GSns.IncGainCnt  = 0;			/* 加速方向のゲイン補正カウンタ */
	bk_b->GSns.DecGainCnt  = 0;			/* 減速方向のゲイン補正カウンタ */
}


/******************************************************************************
【処理】温度センサ バックアップ変数初期化
【戻値】なし
【解説】温度センサのバックアップ変数のうち、基本部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitTSnsBackupData(VOID)
{
	INT i = 0;

	/* オフセット変数初期化 */
	for (i = 0; i<TSNS_OFFSET_MAX; i++)  {
		TSnsBackupData.GyroOfsAve[i] = 0;	/* ジャイロオフセット平均化カウンタ */
		TSnsBackupData.GSnsOfsAve[i] = 0;	/* Gセンサオフセット平均化カウンタ */
		TSnsBackupData.GyroOffset[i] = 0;	/* ジャイロオフセット */
		TSnsBackupData.GSnsOffset[i] = 0;	/* Gセンサオフセット */
	}

	/* ゲイン変数初期化 */
	for (i = 0; i<TSNS_GAIN_MAX; i++)  {
		TSnsBackupData.GyroGainAve[i] = 0;		/* ジャイロゲイン平均化カウンタ */
		TSnsBackupData.GSnsGainAve[i] = 0;		/* Gセンサゲイン平均化カウンタ */
		TSnsBackupData.GyroGain[i] = (WORD)(1.0F * GAIN2WORD);	/* ジャイロゲイン */
		TSnsBackupData.GSnsGain[i] = (WORD)(1.0F * GAIN2WORD);	/* Gセンサゲイン */
	}
}


/******************************************************************************
【処理】センサバックアップ変数初期化
【戻値】なし
【解説】センサのバックアップ変数のうち、拡張部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	)
{
	/* 車速パルス */
	InitSpeedBackupExtension(bk_e);

	/* ジャイロセンサ */
	InitGyroBackupExtension(bk_e);

	/* Gセンサ */
	InitGSnsBackupExtension(bk_e);
}

/******************************************************************************
【処理】車速パルス バックアップ変数初期化
【戻値】なし
【解説】車速パルスのバックアップ変数のうち、拡張部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSpeedBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	)
{
	/* 車速パルス */
	bk_e->Speed.MapMatchCorr = 1.0F;		/* MM距離補正係数 */
	bk_e->Speed.MapMatchCorrCnt = 0;		/* MM距離補正カウンタ */
	bk_e->Speed.PulseErr = FALSE;		/* 車速パルス異常時(パルス線断線時) */

	/*** 車速パルスフィルタのモード ***/
	/* パルスフィルタがPSW OUTに非接続の場合 */
	if (SnsModelInfo.PulseFilter == FALSE)  {
		bk_e->Speed.PulseMode = LO_PULSE;
	}
	/* それ以外 */
	else  {
		bk_e->Speed.PulseMode = HI_PULSE;
	}

	bk_e->Speed.PeriodLengthErr = FALSE;	/* 距離学習値 異常値フラグ */
	bk_e->Speed.LowSpeedNg = FALSE;		/* 低速車速パルス未発生車両判断フラグ */
	bk_e->Speed.MinimumSpeedCnt = 0;		/*   最低出力速度学習回数 */
	bk_e->Speed.MinimumSpeed = 0.0F;		/*   最低出力速度学習値 [m/sec] */
	bk_e->Speed.LastPrdLen = 0.0F;		/* タイヤ径外径変化検出 前回補正前距離学習値 */
	bk_e->Speed.LastPrdLenCnt = 0;		/*   前回補正前距離学習回数 */
	bk_e->Speed.TyreErrCnt = 0;			/*   検出判断用カウンタ */
	bk_e->Speed.PhantomPrdLen = 0.0F;		/*   裏学習値[m] */
	bk_e->Speed.PulseSignalError = FALSE;	/* 車速パルス異常 */
	bk_e->Speed.PastDivNum = 0x00;		/* 過去分周比(JIS基準、分周比異常車両用) */
}

/******************************************************************************
【処理】ジャイロ バックアップ変数初期化
【戻値】なし
【解説】ジャイロのバックアップ変数のうち、拡張部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitGyroBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	)
{
	INT i = 0;

	/* エクセレント学習クリア */
	bk_e->Gyro.ExcellentQuality = 0;		/* エクセレント補正関数のクオリティ 0:関数なし */
	bk_e->Gyro.ExcellentWeek = -1;		/* エクセレントを学習した週 */
	bk_e->Gyro.ExcellentLimit = 0;		/* エクセレント補正の終了時間 */
	for (i=0; i<=EXCELLENT_DIM; i++)  {
		bk_e->Gyro.ExcellentFunction[i] = 0.0F;	/* エクセレント補正の回帰曲線の係数 3次関数に対応 */
	}
	bk_e->Gyro.ExcellentAve = 0.0F;		/* エクセレント学習をしたときのオフセットの平均 */

	/* ジャイロセンサ */
	bk_e->Gyro.Offset       = GYOFF_DEF;	/* Gyro Offset */
	bk_e->Gyro.DirErr       = ERR_INDEX_MAX;	/* 誤差指数最大値 */
	bk_e->Gyro.DirErrRelief = ERR_INDEX_MAX;	/* 誤差指数最大値 */
	bk_e->Gyro.SigmaLevel   = GYRO_OFF_LEVEL;	/* gyro sensor stop level */

	/* 取付け角度変更検出 */
	for (i=0; i<2; i++)  {
		bk_e->Gyro.GainMovingAve[i] = GAIN_DEF;	/* 最新ジャイロゲイン生値平均値 */
		bk_e->Gyro.SettingChangeCnt[i] = 0;	/* 設置角度変更検出カウンター */
	}
	bk_e->Gyro.SensitivityErrCnt = 0;		/* 感度異常検出カウンタクリア */

	bk_e->Gyro.GyroDeviceError = FALSE;		/* Gyroセンサ異常 */
	bk_e->Gyro.AdcDeviceError  = FALSE;		/* A/Dコンバータ異常 */
	bk_e->Gyro.TsnsDeviceError = FALSE;		/* 温度センサ異常 */

}


/******************************************************************************
【処理】Gセンサ バックアップ変数初期化
【戻値】なし
【解説】Gセンサのバックアップ変数のうち、拡張部分を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitGSnsBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	)
{
	/* Gセンサ */
	bk_e->GSns.SigmaLevel	= GSNS_OFF_LEVEL; /* G-Sensor 停止判定のシグマの閾値 */
	bk_e->GSns.SlopeAngle	= 0.0F;		/* 傾斜角 ( down:[-], up:[+] ) */
	bk_e->GSns.YawThetaCnt	= 0;		/* Yaw方向の取り付け角度学習回数 */
	bk_e->GSns.SettingSideCnt	= 0;		/* 回転方向の取り付け向き学習カウンタ */
	bk_e->GSns.CommonGain	= GSNS_GAIN_DEF;  /* 加減速共通ゲイン */
	bk_e->GSns.InstallPitchAngle = 0.0F;	/* ピッチ方向取り付け角度 */
	bk_e->GSns.Reserve	 	= 0;		/* Reserve */
}


/******************************************************************************
【処理】センサバックアップ変数初期化
【戻値】なし
【解説】センサのバックアップ変数のうち、初期学習状態を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSnsLearnStatus(
	sSnsLearnStatus *bk_l	/* センサバックアップ変数構造体 初期学習状態部分へのポインタ */
	)
{
	bk_l->SnsAllOk	= SNS_RESET_NG;		/* 全センサ使用可能フラグ */
	bk_l->ShopMode	= FALSE;		/* 店頭展示モードフラグ */
	bk_l->SpeedOk	= FALSE;		/* 距離学習値使用可能フラグ */
	/* 未学習でも走行を開始するモデルの場合 */
	if (SnsModelInfo.DefaultMove == TRUE)  {
		bk_l->GyroOk	= TRUE;			/* ジャイロ使用可能フラグ */
	}
	else  {
		bk_l->GyroOk	= FALSE;		/* ジャイロ使用可能フラグ */
	}

	/* HONDAモデルの場合 */
	if (SnsModelInfo.Destination == HONDA)  {
		bk_l->GSnsStatus = GSNS_ANGLE;		/* Gセンサ使用可能フラグ */
	}
	else  {
		bk_l->GSnsStatus = FALSE;		/* Gセンサ使用可能フラグ */
	}

	bk_l->RevLineOk	= FALSE;		/* バック線使用可能フラグ */
	bk_l->YawSetupValue	= CENTER_ANGLE;		/* Yaw方向の取付け角度設定値 */
}


/******************************************************************************
【処理】車速パルス 全データ初期化関数
【戻値】なし
【解説】車速パルス処理で使用する全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitSpeedAllParameters(VOID)
{
	/* 車速パルスBackupデータ 基本部分の初期化*/
	InitSpeedBackupBasic(SnsBkB);
	/* 車速パルスBackupデータ 拡張部分の初期化*/
	InitSpeedBackupExtension(SnsBkE);
	/* 車速パルスグローバルデータの初期化 */
	InitSpeedPrms(CHECK_OFF);

	/* CanBus車速パルス異常検出用関連の全データを初期化する */
	InitCanPulseErr();
	/* 速度に対する距離学習値関連の全データを初期化する */
	InitPeriodLengthEx();

	/* 距離学習値近似曲線関連の全データを初期化する */
	InitApxPeriodLengthData();

	/* 距離学習値使用可能フラグ 初期化 */
	SnsBkL->SpeedOk = FALSE;
	/* リバース線使用可能フラグ 初期化 */
	SnsBkL->RevLineOk = FALSE;
}

/******************************************************************************
【処理】車速パルス処理の初期化 + ハード設定 関数 
【戻値】なし
【解説】車速処理関連の変数の初期化とハードウェア設定を行う関数
ハードウェア設定を行うと1ブロック分無効値となるので使用する場合には考慮する事
******************************************************************************/
VOID NPSnsHandlerBase::InitSpeedPrms(
									 BYTE code
									 )
{
	//com.pset
	///* 車速パルス系変数の初期化処理 */
	//InitSpeedVariable(code);

	///* パルスフィルタの設定 */
	//SetPulseMode(SnsBkE->Speed.PulseMode);
}

/******************************************************************************
【処理】ジャイロ全データ初期化関数
【戻値】なし
【解説】ジャイロ処理で使用する全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitGyroAllParameters(VOID)
{
	/* ジャイロBackupデータ 基本部分の初期化 */
	InitGyroBackupBasic(SnsBkB);
	/* ジャイロBackupデータ 拡張部分の初期化*/
	InitGyroBackupExtension(SnsBkE);
	/* ジャイログローバルデータの初期化 */
	InitDirectPrms(0, -1);

	/* ジャイロ使用可能フラグ */
	SnsBkL->GyroOk = FALSE;
}


/******************************************************************************
【処理】Gセンサ全データ初期化関数
【戻値】なし
【解説】Gセンサ処理で使用する全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitGSnsAllParameters(VOID)
{
	/* GセンサBackupデータ 基本部分の初期化 */
	InitGSnsBackupBasic(SnsBkB);
	/* GセンサBackupデータ 拡張部分の初期化*/
	InitGSnsBackupExtension(SnsBkE);
	/* Gセンサローカル変数初期化 */
	InitAccelPrms();

	/* Gセンサ学習状態 */
	if (SnsModelInfo.Destination == HONDA)  {
		/* HONDAモデルでは、3D学習終了 */
		SnsBkL->GSnsStatus = GSNS_ANGLE;
	}
	else  {
		/* 未学習 */
		SnsBkL->GSnsStatus = FALSE;
	}
}


/******************************************************************************
【処理】温度センサ全データ初期化関数
【戻値】なし
【解説】温度センサ処理で使用する全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitTSnsAllParameters(VOID)
{
	/* 温度センサ バックアップ変数初期化 */
	InitTSnsBackupData();

	//com.pset
	///* 温度センサローカル変数初期化 */
	//InitTSnsPrms();
}

/******************************************************************************
【処理】傾斜旋回補正の誤差見積もり計算関連全データ初期化関数
【戻値】なし
【解説】傾斜旋回補正の誤差見積もり計算関連全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitIncEstErrorParameters(VOID)
{

	/* センサ方位誤差積算[DEG] */
	SnsBkIncEstErr.DegreeOfSnsDirErrorPosi = 0.0F;

	/* センサ方位誤差積算(絶対値)[DEG] */
	SnsBkIncEstErr.DegreeOfSnsDirErrorPosiAbs = 0.0F;

	/* センサ方位誤差積算[DEG] */
	SnsBkIncEstErr.DegreeOfSnsDirErrorNega = 0.0F;

	/* センサ方位誤差積算(絶対値)[DEG] */
	SnsBkIncEstErr.DegreeOfSnsDirErrorNegaAbs = 0.0F;

	/* 推測取付け角度の絶対値 単位[度] */
	SnsBkIncEstErr.SetupAngleDeg = 0.0F;
}

/******************************************************************************
【処理】CanBus車速パルス異常検出用関連全データ初期化関数
【戻値】なし
【解説】CanBus車速パルス異常検出用関連全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitCanPulseErr(VOID)
{

	/* 分周比異常レベル(JIS基準) */
	CanPulseErr.DivNumErrLevel = 0;

	/* 非線形性レベル */
	CanPulseErr.NonlinearLevel = 0;

	/* 階段状レベル */
	CanPulseErr.StepwiseLevel = 0;

	/* 交互変化レベル */
	CanPulseErr.AlternusLevel = 0;

	/* Reserve */
	CanPulseErr.Reserve1 = 0;

	/* Reserve */
	CanPulseErr.Reserve2 = 0;

	/* Reserve */
	CanPulseErr.Reserve3 = 0;

	/* Reserve */
	CanPulseErr.Reserve4 = 0;

}

/******************************************************************************
【処理】速度に対する距離学習値関連全データ初期化関数
【戻値】なし
【解説】速度に対する距離学習値関連の全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitPeriodLengthEx(VOID)
{
	INT i = 0;

	for (i=0;i<PRD_LEN_EX_BUF_SIZE;i++)  {
		/* 距離学習値格納バッファ */
		PeriodLengthEx[i].PrdLength = 0.0F;

		/* 距離学習時の平均速度 */
		PeriodLengthEx[i].SpeedAve = 0.0F;

		/* 各速度の距離学習回数 */
		PeriodLengthEx[i].PrdLengthCnt = 0;
	}

}

/******************************************************************************
【処理】距離学習値近似曲線関連全データ初期化関数
【戻値】なし
【解説】距離学習値近似曲線関連の全データの初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitApxPeriodLengthData(VOID)
{
	INT i = 0;

	/* 距離学習値近似曲線係数格納バッファ */
	for (i=0;i<PRD_LEN_CORR_BUF_SIZE;i++)  {
		ApxPeriodLengthData.PrdLengthCorr[i] = 0.0F;
	}

	/* 距離学習値近似曲線算出ステータス */
	ApxPeriodLengthData.PrdLengthProcess = 0;
	/* 距離学習値近似曲線算出使用バッファ数 */
	ApxPeriodLengthData.UseBufCnt = 0;
	/* 距離学習値近似曲線使用可能フラグ */
	ApxPeriodLengthData.PrdLengthOk = 0;

}

/******************************************************************************
【処理】センサバックアップ内GPSバックアップデータ初期化関数
【戻値】なし
【解説】センサバックアップ内のGPSバックアップデータを初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitGpsBackup(VOID)
{
	BYTE	i=0;		    /* ローカルカウンタ */

	// 一度も測位もGPSデータの受信もしていない場合
	if ((!Have1stFix) && (!HaveGpsData)) {
		// 初期値を設定しておく
		bkCurrPosLla[0] = 36.0 * DEG2RAD;	// 緯度
		bkCurrPosLla[1] = 136.0 * DEG2RAD;	// 経度
		bkCurrPosLla[2] = 50;				// 高度
		ComputeS2M(bkCurrPosLla[0]);
	}
	// 何らかのGPS測位データを受信している場合
	else {
		for (i=0; i<3; i++) {
			bkCurrPosLla[i] = GpsMailData.curr_pos_ecef[i];
		}
	}

	bkWeekNumber = -1;		/* 週番号 */
	bkMsecOfWeek = -1;		/* 週時間 */

}

/******************************************************************************
  【処理】緯度、経度の1[sec]当たりの長さ（の逆数）求める
  【戻値】なし
  【解説】距離[m]を緯度、経度に変換する場合、緯度によってその長さが
	  異なるため、変換パラメータが必要になる。
******************************************************************************/
VOID NPSnsHandlerBase::ComputeS2M(
    DOUBLE	latitude	/* 緯度 [sec] */
    )
{
    DOUBLE	sinlat = 0.0;
    DOUBLE	c = 0.0;

    sinlat = sin(latitude);
    c = 1.0 - (bkEcc2 * sinlat * sinlat);
    M2SecLon = RAD2SEC / (bkLocalAxis * cos(latitude) / sqrt(c));
    M2SecLat = 1.0 / ((PI/648000.0) * bkLocalAxis * (1 - bkEcc2)) * sqrt(c*c*c);
    M2SecRefLat = latitude;
}

/******************************************************************************
【処理】センサ変数初期化
【引数】LONG  前回電源OFFからの経過時刻［秒］ 不明な場合は0を引数とする
SHORT  起動時のGPS週番号 不明な場合は-1を引数とする
【戻値】なし
【解説】センサ処理のグローバル変数、ファイル内スタティック変数を初期化する
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorVariables(
	LONG elapsed_time,		/* 前回電源OFFからの経過時刻 ［秒］*/
	SHORT gps_week_num		/* 起動時のGPS週番号 */
	)
{
	//com.pset


	///* センサメイン初期化関数 */
	///* 最初にこの関数を呼ぶこと */
	//InitSensorMain();

	///* センサBIOS用変数初期化 */
	//InitSnsBiosPrms();

	///* LOC 用メールデータの作成関数の変数を初期化する */
	//InitSnsMailDataPrms();

	///* 温度センサー処理の変数を初期化する */
	//InitTSnsPrms();

	///* ジャイロセンサ用変数初期化 */
	//InitDirectPrms(elapsed_time, gps_week_num);

	///* 車速パルス用変数初期化 */
	//InitSpeedPrms(CHECK_ON);

	///* 加速度センサ用変数初期化 */
	//InitAccelPrms();

	///* GPSに利用するセンサデータを初期化する */
	//InitSnsUsable();

	///* CAN-BUS 取得情報 初期化関数 */
	//InitCanBusPrms();

}


/******************************************************************************
【処理】電源ON時のセンサ初期化関数
【戻値】なし
【解説】電源ON時にはこの関数を実行する
******************************************************************************/
VOID NPSnsHandlerBase::InitSensor(VOID)
{
	LONG elapsed_time = 0;  /* 前回電源OFFからの経過時間 */
	SHORT gps_week_num = 0;  /* 起動時のGPS週番号 */

	/* モデル別の動作状態をセットする */
	SetSnsModelInfo();

	//com.pset
	///* 前回電源OFFからの経過時間を計算する */
	//elapsed_time = GetPowerOffTime(&gps_week_num);

	///* センサ処理のグローバル変数、ファイル内スタティック変数を初期化する */
	//InitSensorVariables(elapsed_time, gps_week_num);

	///* センサローレベルハンドラを動作許可する */
	//SetEnableSnsHandle(NP_TRUE, GetMinSpThresh());
}

/******************************************************************************
【処理】センサメイン初期化関数
【戻値】なし
【解説】センサ処理メイン関数の初期化
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorMain(VOID)
{
	/* センサメインのバックアップデータをチェックする */
	MainBackupCheck();

	/* センサ初期学習状態用構造体へのポインタをセット */
	SnsBkL = &SnsLearnStatus;
	/* センサバックアップ変数構造体 基本部分へのポインタをセット */
	SnsBkB = &SensorBackupBasic;
	/* センサバックアップ変数構造体 拡張部分へのポインタをセット */
	SnsBkE = &SensorBackupExtension;

	/* センサプログラムで使用するGPSデータ構造体を初期化する */
	GpsInSns.Mail		= &GpsMailData;	/* GPSメールデータ構造体のポインタをセットする */
	GpsInSns.Velocity3D		= 0.0F;		/* 3次元速度 */
	GpsInSns.PitchAngle		= GPS_PITCH_NG;	/* GPS速度のピッチ角 */
	GpsInSns.LastVelocity	= -1.0F;	/* GPSの前回速度 */
	GpsInSns.LastHeading	= 90.0F;	/* GPSの前回方位 */
	GpsInSns.LastFixTime	= 0.0F;		/* GPSの前回測位時刻 */
	GpsInSns.FixContinue	= FALSE;	/* GPS連続測位判定 */
	GpsInSns.DelayTime		= 0.0F;		/* sdp->t_meas_gyro - fix_time[sec] */

	/* GPS前回測位結果を作成するための変数を初期化 */
	TmpGpsLastVelocity = 0.0F;		/* GPS前回速度を作成するための変数 */
	TmpGpsLastHeading  = 90.0F;		/* GPS前回方位を作成するための変数 */
	TmpGpsLastFixTime  = 0.0F;		/* GPS前回測位時刻を作成するための変数 */

	/* センサ移動距離、角度変化量の積算処理用変数を初期化 */
	memset(&SnsDelta, 0, sizeof(SnsDelta));

#if !defined(ID_CODE)
	/* メールデータ用センサ積算値を無効にする */
	SnsBkM.DeltaSumOn = FALSE;
#endif
}


/******************************************************************************
【処理】現在時刻を取得する関数
【戻値】なし
【解説】現在時刻を取得する
******************************************************************************/
VOID NPSnsHandlerBase::GetCurTime(
								  BYTE 	*buf
								  )
{
	/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)

#else   /* !COMP_OPT_FOR_SNS_SIMULATOR */

	NPGpsTimeInfoStr		TimeData;		// 時刻データ構造体
	NPGpsPoseidonDataStr	*pbyAdr = NULL; 	// 共有メモリのポインタ

	// 時刻データ用の変数を初期化
	memset(&TimeData,0,sizeof(TimeData));

	//com.pset
	//// 現在時刻を共有メモリから取得
	//pbyAdr = reinterpret_cast<NPGpsPoseidonDataStr*>(m_cPOS_SharedMemory.GetAddress());

	// 共有メモリからPoseidonデータを読み出す
	memcpy(reinterpret_cast< PBYTE >(&TimeData), &pbyAdr->sTimeInfo,	sizeof( NPGpsTimeInfoStr ));

	// 現在時刻をセット
	buf[0] = TimeData.CurrTime[1];	// year
	buf[1] = TimeData.CurrTime[2];	// month
	buf[2] = TimeData.CurrTime[3];	// day
	buf[3] = TimeData.CurrTime[4];	// hour
	buf[4] = TimeData.CurrTime[5];	// min
	buf[5] = TimeData.CurrTime[6];	// sec

#endif	/* COMP_OPT_FOR_SNS_SIMULATOR */
}

/******************************************************************************
【処理】センサメインのバックアップデータをチェックする
【戻値】なし
【解説】センサメインのバックアップデータをチェックする
******************************************************************************/
VOID NPSnsHandlerBase::MainBackupCheck(VOID)
{
	/* センサメモリがA,B以外の値になっていないかチェック */
	if ((SnsMemory != GYRO_MEM_A) && (SnsMemory != GYRO_MEM_B))  {
		/* センサのバックアップ変数全てを初期化する */
		/* この関数は、InitSensorVariablesの最初にコールされるので */
		/* グローバル変数の初期化は必要ない */
		InitSensorBackupAll();

		/* バックアップ変数異常 */
		MakeErrorInfo2(71, 0, __LINE__, VERSION);

		/* 関数終了 */
		return;
	}
}

/******************************************************************************
【処理】エラー履歴 SRAM保存処理関数
【戻値】なし
【解説】センサハンドラのエラー履歴をSRAMへ保存する
******************************************************************************/
VOID
NPSnsHandlerBase::WriteToErrInfoSRAM( VOID )
{
#ifdef COMP_OPT_FOR_MACHINE

	/*** !!!注意事項!!! ***/
	// 本関数はコール側で共有メモリに対して排他制御を行っている
	// そのため本関数では排他制御を行わないように注意すること!!

	//com.pset
	//NPSramData  cSram;	// SRAMアクセスクラス

	//// エラー履歴をSRAMに保存
	//if ((cSram.Write(NP_SRAM_AREA_SNS, SNS_ERR_INFO_BK_DATA , m_psDataStatus->sGpsError, sizeof( NPGpsErrorInfoStr ) * 10)) == NP_FALSE)  {
	//	// 失敗
	//	// システムとして動作できないので、エラーログを残す
	//	NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//}

#endif	// COMP_OPT_FOR_MACHINE
}

/******************************************************************************
【処理】センサ学習データ復帰前エラー情報退避関数
【戻値】なし
【解説】センサ学習データ復帰前はエラー履歴も復帰していないので、エラー履歴
復帰時に追加するため、復帰前のエラー情報を退避しておく関数
******************************************************************************/
VOID NPSnsHandlerBase::StoreErrorInfo(
									  BYTE	e_num,		/* エラー番号 */
									  DWORD	value,		/* 任意の値 */
									  DWORD	lines,		/* エラーが発生した行番号 */
									  const CHAR	*id		/* VERSIONで定義したRCS ID文字列へのポインタ */
									  )
{

	//com.pset
	//BYTE	i = 0;

	///* センサエラー情報のバッファ位置異常時 */
	//if (StoreErrInfoNum > ERR_INFO_NUM)  {
	//	/* センサ学習データ復帰前エラー情報初期化 */
	//	InitStoreErrorInfo();
	//}

	///* センサ学習データエラー情報のバッファが全て使用されている場合 */	
	//if (StoreErrInfoNum == ERR_INFO_NUM)  {
	//	/* 一番古いバッファのデータを消去し、ソートする */
	//	for (i=0; i<9; i++)  {
	//		/* エラー番号 */
	//		StoreErrInfo[i].ErrNum = StoreErrInfo[i+1].ErrNum;
	//		/* 任意の値 */
	//		StoreErrInfo[i].Value = StoreErrInfo[i+1].Value;
	//		/* エラーが発生した行番号 */
	//		StoreErrInfo[i].Lines = StoreErrInfo[i+1].Lines;
	//		/* id で渡される文字列が82文字以下の場合は残りはNULLで埋められる */
	//		strncpy(StoreErrInfo[i].Id, StoreErrInfo[i+1].Id, sizeof(StoreErrInfo[i].Id));
	//	}
	//	/* エラー情報保存用ポインタ位置を1つ戻す */
	//	StoreErrInfoNum--;
	//}

	///* エラー情報を退避する */
	///* エラー番号 */
	//StoreErrInfo[StoreErrInfoNum].ErrNum = e_num;
	///* 任意の値 */
	//StoreErrInfo[StoreErrInfoNum].Value = value;
	///* エラーが発生した行番号 */
	//StoreErrInfo[StoreErrInfoNum].Lines = lines;
	///* id で渡される文字列が82文字以下の場合は残りはNULLで埋められる */
	//strncpy(StoreErrInfo[StoreErrInfoNum].Id, id, sizeof(StoreErrInfo[StoreErrInfoNum].Id));

	///* エラー情報保存用ポインタ位置を1つ進める */
	//StoreErrInfoNum++;

}

/******************************************************************************
【処理】ジャイロセンサ処理の変数を初期化する
【戻値】なし
【解説】ジャイロセンサ処理の変数を初期化する
*****************************************************************************/
VOID NPSnsHandlerBase::InitDirectPrms(
									  LONG elapsed_time,		/* 前回電源OFFからの経過時間 [秒] */
									  SHORT gps_week_num		/* 起動時のGPS週番号 */
									  )
{
	INT  i = 0;			/* ループカウンタ */
	sSnsRawData *sdp = NULL;	/* センサ生データ 構造体 */
	DWORD curr_sns_timer = 0;	/* 現在のSnsTimer(電源ONからの生データカウンタ) */

	//com.pset
	///* ジャイロのバックアップデータをチェックする */
	//GyroBackupCheck();

	///*** エクセレント学習の変数を初期化する(初期オフセット決定) ***/
	//InitExcellentLearning(elapsed_time, gps_week_num);

	///*** 停止判定の変数を初期化する ***/
	//memset(&StopJudge, 0, sizeof(StopJudge));
	//StopJudge.GyroStopFlag = FALSE;		/* Gyroの停止条件判断 1:stopped, 0:unfixed */
	//StopJudge.GSnsStopFlag = FALSE;		/* G-Sensorの停止条件判断 1:stopped, 0:unfixed */
	//StopJudge.DvdVideoFlag = FALSE;		/* DVD-VIDEO再生判断フラグクリア */
	///* 停止判定レベル補正の時間を初期化する */
	//StopJudge.GyroSigmaOkLimit = GYRO_STOP_OK_TIME;	/* 停止判定時間補正に初期値を入れる */
	//StopJudge.GyroSigmaNgLimit = GYRO_STOP_NG_TIME;	/* 停止判定時間補正に初期値を入れる */
	//StopJudge.LowPulseFlag = FALSE;		/* 4パルス車より小さいまたは簡易ハイブリッドフラグクリア */
	///* 連続停止チェック */
	//StopJudge.StopJudgeTime = SNS_STOP_TIME_INITIAL;	/* 初期停止判定時間 */
	//StopJudge.StopFlag = FALSE;		/* 停止フラグクリア */
	//StopJudge.PasStopFlag = FALSE;	/* PAS動作中で停止中フラグクリア */

	///* ターンテーブルの変数を初期化する */
	//InitTurnTable();

	///* ジャイロオフセット値用変数初期化 */
	//GyroOffsetFlag = FALSE;		/* initialize Gyro offset flag */
	//GyroDriftFlag = TRUE;		/* ドリフトフラグをドリフト中にセット */
	//GyroOffsetJudgeCnt = 0;		/* gyroオフセット値平均化処理用カウンタクリア */
	//DecrementJudgeCnt = 0;		/* GyroOffsetJudgeCntをデクリメントするためのカウンタ */
	///* オフセット補正変数初期化 */
	//OffsetCorrSource = NO_CORR;
	//LastOffsetCorrSource = NO_CORR;

	///* 取付け位置判別用の変数初期化 */
	//NaviStopErrCnt = 0;			/* 取付け位置NGの回数 */
	//NaviSetErrCnt = 0;			/* 停止判定NGの回数 */
	//NaviSetErrFlag = FALSE;		/* 停止判定NGフラグ */

	///* ジャイロ電圧異常判断用カウンタクリア */
	//GyroVoltageErrCnt = 0;

	///*** ダイナミックオフセット補正構造体を初期化する ***/
	//memset(&DynamicOffset, 0, sizeof(DynamicOffset));
	//DynamicOffset.LastOffset = SnsBkE->Gyro.Offset;	/* 停止直前のオフセット */
	//DynamicOffset.SampleData.GetNewSample = FALSE;	/* 新しいオフセットを取得したことを示すフラグ */
	//DynamicOffset.SampleData.MaxSampleNum = MAX_SAMPLE_DYNAMIC;	/* サンプリングする最大値 */
	//DynamicOffset.HaveRegressLine = FALSE;		/* 回帰直線が計算済みかをしめすフラグ */
	//DynamicOffset.Ready = FALSE;			/* ダイナミックオフセット補正実行可能フラグ */

	///*** GPS方位補正用変数を初期化する ***/
	///* 前回のジャイロ方位初期化(50ms毎、2秒分) */
	//for (i=0; i<DIR_CORR_BUF; i++)  {
	//	GyroDirectBuf[i] = 0.0F;
	//}
	///* GyroDirectBuf内の最新のジャイロ方位の場所をクリア */
	//DirCorrPtr = 0;
	///* ブロックデータの方位変化量バッファ初期化 */
	//for (i=0; i<SNS_BUF; i++)  {
	//	BlockDirBuf[i] = 0.0F;
	//}

	///* GPS方位補正用構造体を初期化する */
	//memset(&GpsDirCorr, 0, sizeof(GpsDirCorr));

	///* GPS方位補正時のオフセット補正用変数初期化 */
	//GyroOfsCorrDir  = 0;		/* 方位補正によるオフセット補正の向き */
	//GyroOfsCorrCnt  = 0;		/* オフセット補正用カウンタ(GPS方位補正時) */
	//FirstGpsDirCorr = FALSE;		/* GPS初期方位補正 未実行 */

	///* GPS方位補正レベルの初期化 */
	//GpsDirCorrLevel = DIR_CORR_LEVEL0;
	///* GPSオフセット補正フラグクリア */
	//GpsOffsetCorr = FALSE;

	///***** MM道路補正用変数初期化 *****/
	//DeltaThetaPtrMm = 0;		/* ジャイロ変化量格納ポインタ(MM用) */
	//for (i=0; i<DIR_MM_BUF; i++)  {
	//	DeltaThetaBufMm[i] = 0.0F;	/* 道路方位補正用ジャイロ変化量格納バッファ */
	//}

	///* MM方位補正レベルの初期化 */
	//MmDirCorrLevel = DIR_CORR_LEVEL0;

	///*** 方位補正によるオフセット補正用変数初期化 ***/
	///* ジャイロ角速度誤差構造体を初期化する */
	//memset(&AngularVelErr, 0, sizeof(AngularVelErr));
	///* 角速度平均化リセットフラグをセット */
	//AngularVelErr.ResetCounter = TRUE;
	///* クイックスタートオフセットリセットフラグ */
	//AngularVelErr.QuickReset = FALSE;


	///* 電源起動時はジャイロ誤差指数を道路方位補正可能な値にする */
	//if (SnsBkE->Gyro.DirErr < MM_DIR_ERR_LIMIT)  {
	//	SnsBkE->Gyro.DirErr = MM_DIR_ERR_LIMIT;
	//}
	//if (SnsBkE->Gyro.DirErrRelief < MM_DIR_ERR_LIMIT)  {
	//	SnsBkE->Gyro.DirErrRelief = MM_DIR_ERR_LIMIT;
	//}

	///*** ジャイロオフセットによる自動方位補正用変数初期化 ***/
	///* 現在のセンサ生データのポインタ取得 */
	//sdp = GetCurrentSnsRawData();
	///* 現在のセンサ生データのSnsTimerを取得 */
	//curr_sns_timer = sdp->SnsTimer;
	///* 自動方位補正構造体に現在の値を代入する(クイックスタート対策) */
	//for (i=0; i<3; i++)  {
	//	OfsDirCorr[i].Time = curr_sns_timer;
	//	OfsDirCorr[i].Offset = SnsBkE->Gyro.Offset;
	//}
	///* 停止中の最後のオフセットをクリア */
	//TmpLastOffset.Time = curr_sns_timer;
	//TmpLastOffset.Offset = SnsBkE->Gyro.Offset;
	///* オフセットによる方位補正要求フラグ */
	//ReqOfsDirCorr = FALSE;
	///* 最終方位補正後に補正したオフセットをクリア */
	//CorredOffsetSum = 0.0F;

	///***** ジャイロゲイン補正用変数初期化 *****/
	//memset(&GyroGain, 0, sizeof(GyroGain));
	//GyroGain.StartGainCorr = FALSE;		/* ゲイン補正開始フラグ */
	//GyroGain.Reservation = FALSE;		/* ゲイン補正保留フラグ */
	///* ゲイン補正要求初期化 */
	//GyroGain.TempDeltaTheta.ReqCorr[DIR_LEFT] = FALSE;
	//GyroGain.TempDeltaTheta.ReqCorr[DIR_RIGHT] = FALSE;
	//GyroGain.Corr.DeltaTheta.ReqCorr[DIR_LEFT] = FALSE;
	//GyroGain.Corr.DeltaTheta.ReqCorr[DIR_RIGHT] = FALSE;
	//GyroGainFlag = FALSE;			/* ゲイン補正実行フラグ */
	//GpsSpeedFlag = FALSE;			/* GPS速度判定用flag clear */
	//GpsMultiPath = FALSE;			/* GPSマルチパス判定 */

	///* ジャイロ感度異常検出用変数初期化 */
	//GyroSensitivityErrFlag = FALSE;		/* ジャイロ感度異常検出フラグクリア */
	//GyroSensRegularDetect = FALSE;		/* ジャイロ感度通常範囲内フラグ初期化 */

	///* ジャイロによる上下面反転取り付け検出用変数初期化 */
	//InitUpsideDownPrms();

	///* 取付け異常フラグクリア */
	//SettingChangeGyro.AngleError = FALSE;

	///* 取付け角度変更フラグ初期化 */
	//SettingChangeGyro.AngleChange = FALSE;

	///* 傾斜ゲイン補正の条件判断の為左右ゲイン学習回数の和を更新する */
	//GyroGain.NGainLRCnt = (WORD)(SnsBkB->Gyro.RGainCorrCnt + SnsBkB->Gyro.LGainCorrCnt);

	///* 推測取付け角度の絶対値[度]の初期化 */
	//SetupAngleDeg = 0.0F;

	///* 温度センサ学習コントロール */
	//HaveTsnsOffset = FALSE;		/* 温度特性学習用のオフセットがあるか？ */
	//SaveTsnsOffset = 0.0F;		/* 温度特性学習用のオフセット保存変数 */
	//TSnsOfsCorrFlag = TRUE;		/* 温度センサオフセット補正 ON/OFF */
	//TSnsOfsSignCheckCnt = 0;		/* 温度センサオフセット値符号カウンタ */

	///* 前回のGPS方位補正時のGPS方位をエラー値で初期化する */
	//LastCorrGpsDir = FALSE_DIR;

	///* 温度センサによるゲイン補正実行用タイマー及びフラグを初期化する */
	//TsnsGyroGainCorr.Flag = TRUE;		/* 初回に実効するのでTRUEで初期化 */
	//TsnsGyroGainCorr.Timer = 0;			/* タイマー初期化 */

	///* 温度センサによるゲイン補正係数及び前回補正時の温度を初期化する */
	//TsnsGyroGainCorr.Rate = 1.0F;					/* 補正係数初期化 1.0F*/
	//TsnsGyroGainCorr.LastCorrTemperature = TEMPERATURE_ERROR_NUM;	/* 前回実行時の温度をエラー値で初期化 */

	///* デバッグ用変数初期化 */
	//RoadDirLog = 0.0F;		/* マッチングフィードバック実行時の道路方位 */
	//MmGyroDir = 0.0F;		/* マッチングフィードバック実行時のジャイロ方位 */
	//GyroDirBuf = 0;		/* ゲイン補正実行時のジャイロ累積方位変化量 */

	///* 駐車場走行判別構造体初期化 */
	//memset(&ParkingJudge, 0, sizeof(ParkingJudge));
	//ParkingJudge.Buffer = 0.0F;		/* Gyro角度変化量積算バッファ */
	//ParkingJudge.Flag = FALSE;		/* 駐車場走行判定フラグ */

	//EstimateDirSetupPosi = 0.0F;	   /* 取り付け誤差が正方向であった場合の方位 */
	//EstimateDirSetupNega = 0.0F;	   /* 取り付け誤差が負方向であった場合の方位 */

	//ReqResetHeadingFlag = NP_FALSE;	   /* 方位リセット要求フラグ */

}

/******************************************************************************
【処理】Gセンサ処理初期化関数
【戻値】VOID
【解説】Gセンサ処理で使用する変数等の初期化を行う
******************************************************************************/
VOID NPSnsHandlerBase::InitAccelPrms(VOID)
{
	INT  i = 0;

	//com.pset
	///* Gセンサ Backup Data Check */
	//GsnsBackupCheck();

	///*** Static変数を初期化する ***/
	//GsnsAngleSet	= FALSE;
	//GsnsBack		= G_FORWARD;
	//SlopeFlag		= SLOPE_0;
	//SuperSlope		= FALSE;
	//Pls0Cnt 		= 0;
	//GsnsDirErrCnt 	= 0;
	//AccelStatus 	= ACCEL_0;
	//GsnsSpeed 		= 0.0F;
	//AngleBufPtr 	= 0;		/* 傾斜角格納ポインタ */
	//DeltaDirPtr 	= 0;
	//PulseBufPtr 	= 0;
	//PulseSum200ms	= 0;		/* パルス数の合計 */
	//GsnsAccelPtr 	= 0;		/* Gセンサ加速度格納ポインタ */
	//GsnsAcAvPtr 	= 0;		/* Gセンサ加速度平均値格納ポインタ */
	//GsnsAccelAv 	= 0.0F;		/* Gセンサ加速度平均値 */
	//PulseAccelAv	= 0.0F;		/* Pulse加速度平均値 */
	//CheckBackFlag 	= FALSE;	/* バック状態を監視するためのフラグ */
	//CheckBackTimer 	= 0;		/* バックを検出した後の時間 */
	//BackJudgTimer 	= 0;
	//GsnsAdPtr 		= 0;		/* AD値格納ポインタ */
	//DirJudgeCnt 	= 0;		/* Gセンサの設置方向判別用カウンター */
	//GsnsForwardCnt 	= 0;		/* 1回の加速と判定した回数 */
	//StopAdCnt 		= 0;		/* 停止時のAD値格納ポインタ */
	//Pls0BackFlag 	= NOT_VALUE;		
	//PulseAcFlag 	= FALSE;	/* パルス加速度算出フラグ */
	//GpsAcFlag	 	= FALSE;	/* GPS加速度算出フラグ */
	//LastSlopeAngle 	= SnsBkE->GSns.SlopeAngle;
	//PulseSpPtr		= 0;
	//GsnsAdAv		= SnsBkB->GSns.Offset;  /* Gセンサ出力の平均値 */ 
	//GsnsAdStop		= SnsBkB->GSns.Offset;	/* 停止中のGセンサA/D値 */
	//GsnsAdDifMax	= 0.0F;		/* 停止中のAD値と停止用平均AD値との差分 */
	//AccelLimit		= 0.0F;		/* バック判定用の閾値 */
	//DirCheckCnt		= 0;		/* 取り付け方向誤り判定回数 */	
	//GsnsAccelGpsDelay	= 0.0F;		/* GPS加速度と同期したGセンサ加速度[m/s^2] */
	//LastGpsFixTime	= 0.0F;		/* 前回傾斜角を算出したGPS測位時刻 */
	//TmpGsnsSpeed	= 0.0F;		/* 距離補正用速度退避変数 */
	//AccelSumPtr		= 0;		/* パルス加速度積算用ポインタ */
	//OffsetStartCnt	= 0;		/* 電源ONからのオフセット学習回数 */
	//AccelSumFlag 	= FALSE;	/* 加速度初期格納終了フラグ */
	//GsnsSpeedReset	= FALSE;	/* Gセンサ速度リセット */
	//GsnsAccelGain	= 0.0F;		/* ゲイン学習用Gセンサ加速度 */
	//PulseAccelGain	= 0.0F;		/* ゲイン学習用パルス加速度 */
	//GpsAccel		= 0.0F;		/* Gps加速度 */
	//DispAccel		= 0.0F;		/* 画面表示用の加速度 */
	//GsnsAccelAvDelay	= 0.0F;		/* Gセンサ加速度 */
	//DeltaThetaAbs	= 0.0F;		/* 角度変化量の絶対値 */
	//NoPulseAngle.Flag	= FALSE;	/* 傾斜角使用可能フラグ */
	//NoPulseAngle.Timer	= 0;		/* 傾斜角補正用タイマー */
	//PitchErrorCnt	= 0;		/* Pitch角度変更監視カウンタ */
	//PitchChangeCnt	= 0;		/* Pitch角度変更監視カウンタ */
	//SettingChangeGSns.AngleError  = FALSE; /* 取り付け角度異常検出 */ 
	//SettingChangeGSns.AngleChange = FALSE; /* 取り付け角度変更検出 */ 
	//OffsetCycle.Flag	= FALSE;	/* 補正動作周波数設定フラグ */
	//OffsetCycle.Timer	= 0;		/* 補正動作周波数設定タイマー */
	//GsnsAd1sPtr		= 0;		/* AD平均値格納ポインタ */
	//SlopeAngleGyro 	= SnsBkE->GSns.SlopeAngle;  /* Gyro角度補正用傾斜角 */
	//GSnsAdSum		= 0.0F;		/* AD値の移動平均値 */
	//AdSumFlag		= FALSE;	/* AD値初期格納終了フラグ */
	//AdSumPtr		= 0;		/* AD値格納アドレス */
	//PulseAcSum		= 0.0F;		/* パルス加速度の積算値 */
	//GsnsAcSum		= 0.0F;		/* Gセンサ加速度の積算値 */
	//AngleSum		= 0.0F;		/* 傾斜角の積算値 */
	//FirstOffsetTsns 	= FALSE;	/* 初期オフセット温度補正フラグ */
	//TSnsGOfsLearnTimer	= 0;		/* 温度毎のオフセット学習タイマー */
	//SumDeltaTheta	= 0.0F;		/* 1s間の角度変化量 */
	//GainCorrTimer	= 0;		/* ゲイン補正実行タイマー */
	//AccelErr		= 0;		/* 加速度異常カウンタ */
	//AccelErrTimer	= ACCEL_ERR_TIME;	/* 連続性監視用カウンタ */

	///* 傾斜角格納バッファ */
	//for (i=0; i<ANGLE_BUF; i++) {
	//	AngleBuf[i] = SnsBkE->GSns.SlopeAngle;
	//}
	///* 角度変化量格納バッファ */
	//for (i=0; i<F_SNS; i++) {
	//	DirBufGsns[i] = 0.0F;
	//}
	///* パルス数格納バッファ */
	//for (i=0; i<PLS_SUM_BUF; i++) {
	//	PlsCntSumBuf[i] = 0;
	//}
	///* Gセンサ加速度格納バッファ */
	//for (i=0; i<G_AC_BUF; i++) {
	//	GsnsAccelBuf[i] = 0.0F;
	//}
	///* Gセンサ加速度平均値バッファ */
	//for (i=0; i<AC_DELAY_BUF; i++) {
	//	GsnsAcAvBuf[i] = 0.0F;
	//}
	///* 停止時のAD値格納バッファ */
	//for (i=0; i<STOP_AD_BUF; i++) {
	//	StopAdBuf[i] = SnsBkB->GSns.Offset;
	//}
	///* Pulse速度格納バッファ: 1秒分 */
	//for (i=0; i<PULSE_SP_BUF; i++) {
	//	PulseSpBuf[i] = PULSE_ERR;
	//}
	///* AD値格納バッファ */
	//for (i=0; i<GSNS_AD_BUF; i++) {
	//	GsnsAdBuf[i]   = SnsBkB->GSns.Offset;
	//	GsnsAd1sBuf[i] = SnsBkB->GSns.Offset;
	//}
	///* パルス無し時のGセンサA/D値 : 500-600ms */
	//for (i=0; i<PLS0_AD_BUF; i++) {
	//	Pls0AdBuf[i] = SnsBkB->GSns.Offset;
	//}
	///* パルス加速度積算用バッファ  */
	///* Gセンサ加速度積算用バッファ */
	///* 傾斜角積算用バッファ        */
	//for (i=0; i<GAIN_SUM_TIME; i++) {
	//	PulseAcSumBuf[i] = 0.0F;
	//	GsnsAcSumBuf[i]  = 0.0F;
	//	AngleSumBuf[i]   = 0.0F;
	//}
	///* AD値格納バッファ */
	//for (i=0; i<AD_SUM_TIME; i++) {
	//	AdSumBuf[i] = GSNS_OFS_DEF;
	//}

	///*** オフセット学習カウンタを電源起動時の回数へ戻す ***/
	//if (SnsBkB->GSns.OffsetCnt > OFS_START_RATE) {
	//	SnsBkB->GSns.OffsetCnt = OFS_START_RATE;
	//}

	///* 平均のゲイン補正値を作成する */
	//SnsBkE->GSns.CommonGain = 0.5F * (SnsBkB->GSns.IncGain + SnsBkB->GSns.DecGain);

	///* 傾斜角用の変数に退避する */
	//StoreAngleParameters();

}



/******************************************************************************
【処理】センサバックアップデータSRAM読み込み処理関数
【戻値】なし
【解説】SRAMからセンサのバックアップの読み込みを行う
******************************************************************************/
VOID
NPSnsHandlerBase::ReadToSnsBkSRAM( VOID )
{
#ifdef COMP_OPT_FOR_MACHINE

	//com.pset
	//NPSramData  cSram;	// SRAMアクセスクラス
	NP_BOOL	ret = NP_FALSE;	// SRAM読み込み結果
	PBYTE	start_add = NULL;	// 書き込み開始位置
	NPGpsErrorInfoStr err_info[10];	/* エラー履歴初期化用バッファ */

	// 書き込み開始アドレスの設定
	start_add = (PBYTE)&SNS_SIGNWORD;

	// 書き込み先を、0フィルしておく
	memset(start_add, 0, AdjSnsMemSize);

	// SRAMへのアクセス
	// センサ学習データ Aセクションの読み込み
	ret = ReadToASectionSnsBkSRAM();

	// Aセクションが読み込めない場合
	if (ret == NP_FALSE) {
		// Bセクションの読み込みを行う
		ret = ReadToBSectionSnsBkSRAM();
	}

	// 学習データを読み込めた場合の処理
	if (ret == NP_TRUE) {
		// センサバックアップデータ利用可能
		SnsMemCondition = NP_SNS_MEM_CONDITION_OK;
		/* デコード処理で必要な変数の初期化を行う */
		InitSensorDecodeVariable();
		/* センサのバックアップのチェックサムを計算 */
		AddSnsBackupCheckSum(NP_TRUE);

		// SRAMからエラー履歴の復活を行う
		// エラー履歴が正常が判定する方法がないので、
		// センサ学習データが正常な場合にはSRAMからの復帰をしてしまう
		ReadToErrInfoSRAM();

		/* エラー履歴復帰処理フラグを立てる */
		ErrorInfoCondition = NP_TRUE;
		/* センサ学習データ復帰前エラー情報を追加 */
		AddErrorInfo();

		// 処理時間計測用ログ出力
		OutputTimeRETAILMSG( NP_SNS_BK_MEM_FROM_SRAM , 0x00);

	}
	// 学習データが読み込めなかった場合の処理
	else {
		// 書き込み先を、0フィルしておく
		memset(start_add, 0, AdjSnsMemSize);

		// センサバックアップデータを使用不可とする
		SnsMemCondition = NP_SNS_MEM_CONDITION_NG;
		/* センサのバックアップ変数全てを初期化する */
		InitSensorBackupAll();
		/* センサのバックアップのチェックサムを計算 */
		AddSnsBackupCheckSum(NP_TRUE);

		// エラー履歴初期化用バッファを0フィルしておく
		memset(&err_info[0], 0, sizeof( NPGpsErrorInfoStr ) * 10);

		//com.pset
		//// エラー履歴を初期化
		//if ((cSram.Write(NP_SRAM_AREA_SNS, SNS_ERR_INFO_BK_DATA , err_info, sizeof( NPGpsErrorInfoStr ) * 10)) == NP_FALSE )  {
		//	// 失敗
		//	// システムとして動作できないので、エラーログを残す
		//	NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
		//}

		// 処理時間計測用ログ出力
		OutputTimeRETAILMSG( NP_SNS_BK_MEM_FROM_SRAM , 0xFF);

	}
	/* 学習データが読み込めていない場合、必ずエラー(71)が発生する */
	/* グローバル変数、ファイル内スタティック変数を初期化する */
	InitSensorVariables(0, -1);

#endif	// COMP_OPT_FOR_MACHINE
}

/******************************************************************************
【処理】処理時間計測用データ出力関数
【戻値】なし
【解説】RETAILMSGにて処理時間計測用のログデータを出力する
******************************************************************************/
VOID NPSnsHandlerBase::OutputTimeRETAILMSG(
	unsigned char process_id,	// プロセス名
	DWORD value			// 拡張情報
	)
{
#ifdef COMP_OPT_FOR_MACHINE
	// 処理時間測定用のログを出力する場合
#if defined(COMP_OPT_FOR_TIME_LOG_MODE)
	DWORD	process_time = 0;	/*プロセス時刻 */

	/* プロセス時刻の取得 */
	/* ms単位での時刻を取得 */
	process_time = ::GetTickCount();

	/* デバッグメッセージの出力 */
	RETAILMSG( 1, ( TEXT( "NP_SNS_MSG, TIME_LOG, %d, %d, %d\n" ), process_id, process_time, value ) );

#endif	// COMP_OPT_FOR_TIME_LOG_MODE
#endif	// COMP_OPT_FOR_MACHINE
}

/******************************************************************************
【処理】センサ学習データ復帰前エラー情報追加関数
【戻値】なし
【解説】退避したセンサ学習データ復帰前エラー情報を追加する関数
※エラー番号、任意の値、エラー発生行、ID文字列以外は、
エラー情報追加時のデータが履歴として残る
******************************************************************************/
VOID NPSnsHandlerBase::AddErrorInfo(VOID)
{

	BYTE	i = 0;
	BYTE	e_num = 0;		/* エラー番号 */
	DWORD	value = 0;		/* 任意の値 */
	DWORD	lines = 0;		/* エラーが発生した行番号 */
	CHAR	*id = NULL;		/* VERSIONで定義したRCS ID文字列へのポインタ */

	/* センサ学習データ復帰前エラー履歴が存在しない場合 */
	if (StoreErrInfoNum == 0)  {
		/* 処理を返す */
		return;
	}

	/* 退避されたエラー情報を追加する */
	for (i=0; i<StoreErrInfoNum; i++)  {
		/* エラー番号 */
		e_num = StoreErrInfo[i].ErrNum;
		/* 任意の値 */
		value = StoreErrInfo[i].Value;
		/* エラーが発生した行番号 */
		lines = StoreErrInfo[i].Lines;
		/* VERSIONで定義したRCS ID文字列へのポインタ */
		id = StoreErrInfo[i].Id;

		/* エラーインフォメーション出力 */
		MakeErrorInfo2(e_num, value, lines, id);
	}
}

/******************************************************************************
  【処理】デコード処理用変数初期化関数
  【戻値】なし
  【解説】デコード処理用の変数の初期化を行う関数
******************************************************************************/
VOID NPSnsHandlerBase::InitSensorDecodeVariable(VOID)
{
    /* Gpsデータ受信済みである事をフラグに示す */
    HaveGpsData = NP_FALSE;
}

/******************************************************************************
【処理】エラー履歴 SRAM読み込み処理関数
【戻値】なし
【解説】センサハンドラのエラー履歴をSRAMから読み込む
******************************************************************************/
VOID
NPSnsHandlerBase::ReadToErrInfoSRAM( VOID )
{
#ifdef COMP_OPT_FOR_MACHINE

	//com.pset
	//NPSramData  cSram;	// SRAMアクセスクラス

	//// 排他オブジェクト取得
	//m_cData_SyncObject.Create(NP_GPS_DATA_SYNC);
	//m_cData_SyncObject.SynchronizeStart( );

	//// エラー履歴を復元
	//if ((cSram.Read(NP_SRAM_AREA_SNS, SNS_ERR_INFO_BK_DATA , m_psDataStatus->sGpsError, sizeof( NPGpsErrorInfoStr ) * 10)) == NP_FALSE)  {
	//	// 失敗
	//	// システムとして動作できないので、エラーログを残す
	//	NPRecordErrorN  ( NP_ERROR_SNS_SRAM , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//}

	//// エラー履歴情報受信済みフラグ
	//m_psDataStatus->bGpsReceiveErrorInf = NP_TRUE;

	//// 排他オブジェクトの終了
	//m_cData_SyncObject.SynchronizeEnd();
	//m_cData_SyncObject.Destroy();

#endif	// COMP_OPT_FOR_MACHINE
}

/******************************************************************************

SNSハンドラの初期化

【解説】
SNSハンドラの初期化関数
NPHandlerBase::Initialize()からコールされる

*******************************************************************************/
VOID
NPSnsHandlerBase::OnInitialize(
							   VOID
							   )
{
	/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)
#else   /* !COMP_OPT_FOR_SNS_SIMULATOR */

	BYTE    datum_code = 40;	/* 測地系 */

	// 共有メモリオープン
	//NPGpsPoseidonDataStr *pbyAdr = reinterpret_cast<NPGpsPoseidonDataStr*>(m_cPOS_SharedMemory.Open(NP_GPS_POS_MEMORY));
	//m_psDataStatus = reinterpret_cast<NPGpsStoreDataStatusStr*>(m_cData_SharedMemory.Open(NP_GPS_DATA_MEMORY));

	//// 共有メモリ作成（フィードバックデータ）
	//m_cLOC_Feedback_SharedMemory.CAlloc( NP_GPS_LOC_FEEDBACK_MEMORY, sizeof( NPGpsMatchDataStr ) );
	//// フィードバックデータ用の排他
	//m_cLOC_Feedback_SyncObject.Create( NP_GPS_LOC_FEEDBACK_READ_SYNC );


#ifdef COMP_OPT_FOR_MACHINE
	/* 測地系の設定 */
	//NPGpsHandler::GetGpsHandler()->GetGpsDatumCode(&datum_code);
	//LocalParms(datum_code);

	//// 4byte倍数のセンサバックアップデータサイズの算出
	//AdjustSensorBackupDataSizeCalc();

	// センサ学習データの読み込みを行う 
	ReadToSnsBkSRAM();

#endif	// COMP_OPT_FOR_MACHINE

	//// ハンドラ内のAPIコール用
	//m_pcFeedBack		= new NPSnsFeedBack( this );
	//if ( NULL == m_pcFeedBack ) {
	//	// システムとして動作できないので、エラーログを残して処理終了
	//	NPRecordErrorN  ( NP_ERROR_SNS_LOC_THREAD_ERROR , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//	return;
	//}

	//// カードログスレッド生成
	//m_pcCardLog = new NPSnsCardLog( this );
	//if ( NULL == m_pcCardLog ) {
	//	// システムとして動作できないので、エラーログを残して処理終了
	//	NPRecordErrorN  ( NP_ERROR_SNS_LOG_THREAD_ERROR , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//	return ;
	//}
	//// 初期化処理
	//m_pcCardLog->Initialize();

	//// ファイルの状態  TRUE:開 FALSE:閉
	//m_bLogFileOpen = NP_FALSE;

	//m_eLogCondition = NP_SNS_LOG_CONDITION_OFF;
	//CardLogMode = GPS_TEST_OFF;

	//// ログ開始時刻[sec]
	//m_dwLogStartTime = 0;

	//// カードログの状態を初期化
	//memset( m_szCardLogFileName , 0 , sizeof( m_szCardLogFileName ) );

	//// センサ学習用スレッド生成
	//m_pcSnsLearn = new NPSnsLearn( this );
	//if ( NULL == m_pcSnsLearn ) {
	//	// システムとして動作できないので、エラーログを残して処理終了
	//	NPRecordErrorN  ( NP_ERROR_SNS_LEARN_THREAD_ERROR , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//	return ;
	//}

	//// エラー履歴用スレッド生成
	//m_pcSnsErrInfo = new NPSnsErrorInfo( this );
	//if ( NULL == m_pcSnsErrInfo ) {
	//	// システムとして動作できないので、エラーログを残して処理終了
	//	NPRecordErrorN  ( NP_ERROR_SNS_ERROR_THREAD_ERROR , VERSION, __LINE__, 0, 0, FALSE, NPLOG_IMMEDIATE );
	//	return ;
	//}
#endif  /* COMP_OPT_FOR_SNS_SIMULATOR */
}

NPSnsHandlerBase::NPSnsHandlerBase():
GpsTestMode(0),			/* GPS Test Mode On flag */
SimLog(0),				/* GPS/SNS Data output for Simulation */
RawDataReadPtr(0),			/* SnsRawData構造体リードポインタ */
RawDataWritePtr(0),			/* SnsRawData構造体ライトポインタ */
LastSnsTimer(0),			/* 前回のセンサタイマ */
/* センサシミュレータなら宣言を行う */
#if defined(COMP_OPT_FOR_SNS_SIMULATOR)
SnsDecDebugCnt(0),			/* Debug用 配列カウンタ */
#endif	/* COMP_OPT_FOR_SNS_SIMULATOR */

SnsBkL(NULL),	/* センサ初期学習状態用構造体へのポインタ */
SnsBkB(NULL),	/* センサバックアップ変数構造体 基本部分へのポインタ */
SnsBkE(NULL),	/* センサバックアップ変数構造体 拡張部分へのポインタ */

NotifiedOfGpsFlag(NP_FALSE),      	/* GPS測位データの通知がされていることを示すフラグ */
ErrorInfoCondition(NP_FALSE),	/* エラー履歴復帰状態フラグ */
StoreErrInfoNum(0),			/* センサ学習データエラー情報保存用ポインタ */

SNS_SIGNWORD(SNS_SIGNATURE),	/* SENSOR SIGNWORD */
SnsSRamNow(SNS_SRAM),		/* 現在のSNS-SRAM管理番号 */
SnsMemory(GYRO_MEM_A),		/* 現在使用中のセンサメモリGYRO_MEM_A or GYRO_MEM_B */
SnsBackupError(0),			/* バックアップデータエラーフラグ */
BackupCheckSum(0x00),		/* バックアップデータのチェックサム */
bkWeekNumber(0),			/* 週番号 */
bkMsecOfWeek(0),			/* 週時間 */
sns_bat_sign('N'),			/* signature for battery RAM */
SnsMemCondition(NP_SNS_MEM_CONDITION_NG),	/* センサバックアップデータ状態 */
MakeErrorInfoFlag(NP_FALSE),	/* エラー履歴更新フラグ */
LatestWeekNumber(-1),		// 週番号
LatestMsecOfWeek(-1),		// 週時間
DeltaTheta(0.0F),			/* 1生データ(SNS_BUF * 50ms)間の角度変化量 */
//GpsMultiPath(0),			/* GPSマルチパス判定 */
TmpGpsLastVelocity(0.0F),		/* GPS前回速度を作成するための変数 */
TmpGpsLastHeading(0.0F),		/* GPS前回方位を作成するための変数 */
TmpGpsLastFixTime(0.0F),		/* GPS前回測位時刻を作成するための変数 */
HaveGpsData(NP_FALSE),		/* Gpsデータ受信済みである事をフラグに示す */
AdjSnsMemSize(0)			/* 4BYTE倍数へ調整済みセンサバックアップデータサイズ */
{
}


enum NPGpseHaveSnsMemEnum{
    NP_GPS_HAVE_SNS_MEM_OFF = 0,                    ///< センサメモリ保存データ無し
    NP_GPS_HAVE_SNS_MEM_1,                          ///< センサメモリ保存データ１が有効
    NP_GPS_HAVE_SNS_MEM_2                           ///< センサメモリ保存データ２が有効
} ;

struct NPSnsFlagStatus{
    LONG	lSnsCardLogStatus;
    DWORD	Reserve1;
    DWORD	Reserve2;
    DWORD	Reserve3;
    DWORD	Reserve4;
    DWORD	Reserve5;
} ;

NPGpsErrorInfoStr sss[20];

int main()
{
	NPSnsHandlerBase nphandlerbase;

	nphandlerbase.OnInitialize();

	//cout << sizeof(NPGpsErrorInfoStr) << endl;

	//nphandlerbase.CopySnsBkSRAM(0);
	//cout << sizeof(NPGpseHaveSnsMemEnum) << endl;

	//cout << sizeof(NPSnsFlagStatus) << endl;

	cout << sizeof(sss) << endl;

	return 0;
}