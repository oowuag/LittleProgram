#include "Sns06.SRAM_Gps.h"

#include <Windows.h>
#include <math.h>



#define	SNS_GLOBAL
#define	SNS_GLOBAL_B

#define	GPS_GLOBAL
#define	GPS_GLOBAL_B

////
#define	TSNS_OFFSET_MAX		50		/* 温度センサバックアップ変数の配列最大値 */
#define	TSNS_GAIN_MAX		20		/* 温度センサバックアップ変数の配列最大値 */

#define	PRD_LEN_EX_BUF_SIZE	7		/* 速度に対する距離学習値保存用構造体のバッファ数 */
#define	PRD_LEN_CORR_BUF_SIZE	4		/* 距離学習値近似曲線係数格納バッファ数 */

/*** sns_model.h ***/
/* 仕向け別の動作Modeの定義 */
#define	AFTER			0x00		/* 市販製品 */
#define	TOYOTA			0x01		/* TOYOTA向けOEM */
#define	FORD			0x02		/* FORD向けOEM */
#define	HONDA			0x03		/* HONDA向けOEM */

/*** モデル定義用 ***/
#define	NP_SNS_NAVI_UNKNOWN	0x0000		/* モデル定義不明 */
#define	NP_SNS_NAVI_OTHER	0x0001		/* 特定の必要なし */
#define	NP_SNS_NAVI_JG500	0x0002		/* JG500/上海汽車 */

/* 取付け角度の定義 1/Cos(deg)で記述すること */
#define	LEVEL			1.0F		/* 水平取り付け */
#define	SLANT			1.03528F	/* 傾斜取り付け(AVN) 1/Cos(15deg) */
#define	HONDA_SFEB		1.07114F	/* 傾斜取り付け(SFEB) 1/Cos(31-10deg) */
#define	HONDA_SJKA		1.00137F	/* 傾斜取り付け(SJKA) 1/Cos(13-10deg) */
#define	HONDA_SJDA		1.04569F	/* 傾斜取り付け(SDJA) 1/Cos(27-10deg) */
#define	HONDA_SDAX_KH		1.06418F	/* 傾斜取り付け(SDAX-KH) 1/Cos(30-10deg) */
#define	HONDA_SFEW		1.07114F	/* 傾斜取り付け(SFEW) 1/Cos(31-10deg) */


/* センサ学習データ復帰前エラー情報用 */
#define	ERR_INFO_NUM		10			/* センサ学習データ復帰前エラー情報用バッファ数 */


/* 数値 */
#ifdef	PI
#   undef PI
#endif
#define	PI			3.1415926535897932L	/* π */

#define	DEG2RAD			(PI/180.0)		/* degからラジアンへの変換 */
#define	RAD2DEG			(180.0/PI)		/* ラジアンからdegへの変換 */

#define	RAD2SEC			(RAD2DEG*3600.0)	/* ラジアンから秒への変換 */

#define	BYTE2DEG		(360.0F/256)		/* 360/256degからdegへの変換 */

#define	DEG2WORD		(65536/360.0F)		/* degから360/65536degへの変換 */

#define	MAX_DAY_SEC		86400.0F 		/* Maximum SEC count in one day */
#define	MAX_WEEK_SEC		604800.0F 		/* Maximum SEC count in one week */
							/* 604800 = 7day*24hour*60min*60sec */
							/*        = 1week */
#define	MAX_WEEK_NUM		1024			/* 1 cycle of GPS week number */


/*** CAN-BUSからのEOL設定情報関連 ***/
/* VSS設定関連 */
#define	CAN_EOL_VSS_PTN			0x0000000F	/* EOL設定 VSS設定 */
#define	CAN_EOL_MSCAN_VSS		0x00000001	/* EOL設定 MS-CAN */
#define	CAN_EOL_HARDWIRE_VSS		0x00000002	/* EOL設定 ハードワイヤ(デフォルト) */
#define	CAN_EOL_UNCONFIGURED_VSS	0x00000004	/* EOL設定 未構成 */
#define	CAN_EOL_NOT_USED_VSS		0x00000008	/* EOL設定 未使用 */
/* リバース設定関連 */
#define	CAN_EOL_REV_PTN			0x000000F0	/* EOL設定 リバース設定 */
#define	CAN_EOL_MSCAN_REV		0x00000010	/* EOL設定 MS-CAN */
#define	CAN_EOL_HARDWIRE_REV		0x00000020	/* EOL設定 ハードワイヤ */
#define	CAN_EOL_MSCAN_HARDWIRE_REV	0x00000040	/* EOL設定 MS-CAN & ハードワイヤ(デフォルト) */
#define	CAN_EOL_NOT_USED_REV		0x00000080	/* EOL設定 未使用 */

/*** sns_bkup.h ***/

/* SNSのバックアップ変数を追加、削除、移動した場合は必ず
*  値を書き換えること(書式：0x年月日)
*/
#define	SNS_SRAM		0x080924	/* SNS SRAM 管理番号(最新) */
#define	SNS_SRAM_V001		0x080924	/* SNS SRAM 管理番号(Ver.001時点) */
#define	SNS_SRAM_V002		0x080924	/* SNS SRAM 管理番号(Ver.002時点) */
						/* 周期カウントを+1しているバージョン */
						/* バックアップデータを変更しないため、Ver.001と共用 */

/*** センサハンドラで利用するSRAMエリアについて ***/
// SRAMの 0～8KByteのエリアをセンサハンドラにて利用する
// そのエリアを、センサハンドラ用2つとエラー履歴用1つに分割し利用を行う
// センサハンドラ用は書き込み毎に、AとBの2つのセクションに書き込みを行う
// 2つ持つことで、メモリ忘れ時の保護を行うものとする
// エラー履歴は、エラー発生時に書き込みを行うものとする
// SRAMアクセス用のオフセット値を以下に定義する
#define	SNS_LEARN_BK_DATA_A	0	/* センサハンドラ センサ学習バックアップデータ SRAMアクセス用(Aセクション) */
#define	SNS_LEARN_BK_DATA_B	1024	/* センサハンドラ センサ学習バックアップデータ SRAMアクセス用(Bセクション) */
#define	SNS_ERR_INFO_BK_DATA	2048	/* センサハンドラ エラー履歴バックアップデータ SRAMアクセス用 */




////
#define	NP_SNS_LOG_THREAD_WAKEUP		0x0F	/* スレッド処理開始 */
#define	NP_SNS_LOG_THREAD_SLEEP			0x10	/* スレッド処理終了 */
#define	NP_SNS_OFF_PROC_WAKEUP			0x11	/* OFF処理開始 */
#define	NP_SNS_OFF_PROC_SLEEP			0x12	/* OFF処理終了終了 */
#define	NP_SNS_BK_MEM_FROM_SRAM			0x13	/* SRAMから学習データ復帰処理を行った時刻 */
#define	NP_SNS_BK_MEM_WAIT			0x14	/* 学習データ読み込み待ち処理終了時刻 */

/* SRAMメモリエリアセレクト用定義 */
#define	NP_SNS_A_SECTION	0x00			/* Aセクション */
#define	NP_SNS_B_SECTION	0x01			/* Bセクション */

/* SRAMメモリエリアサイズ */
#define NP_SNS_SECTION_AREA	1024			/* A,Bセクションのエリアサイズ */

/*** gps_diag.h ***/
#define	SIGNATURE97		0x7242		/* TANS Original Value */
/*** tsk_gyro.cpp ***/
#define	SNS_SIGNATURE		SIGNATURE97+0x04	/* '04～ センサーで使用(GIDRAモデル) */


/* リバース線 */
/* リバース線の端子に入力されている電圧の状態を示す。(DRAGON内部では反転するので注意) */
#define	REV_SNS_L		0		/* リバースギア端子  0[V]時のプログラム内論理 */
#define	REV_SNS_H		1		/* リバースギア端子 12[V]時のプログラム内論理 */
#define	REV_UNKNOWN		2		/* 前進論理不明 */

#define	ADS7828			0x04		/* TI社 8ch 50kHz */
#define	XV_8100CB		0x0A		/* エプソントヨコム */
#define	LIS244ALH		0x08		/* STMicro */
#define	S_58LM20A		0x03		/* セイコーインスツル */

/* 取り付け位置変更 補正機能 */
#define	NON_CORR		0		/* 補正機能OFF */
#define	AUTO_CORR		1		/* 補正をDRAGON側で自動的に実行 */
#define	MANUAL_CORR		2		/* 補正をアプリ(ユーザー)の返答結果により実行 */

/* 車両実装時のGセンサ取付け向き */
#define	UNKNOWN_SET		0		/* 不定 */
#define	REVERSE_SET		1		/* 反転取付け */
#define	FRONT_SET		2		/* 正面取付け */

/* 車速パルス信号情報源 / リバース信号情報源 */
#define	HARD_WIRED_ONLY		0		/* ハードワイヤのみ */
#define	CAN_BUS_ONLY		1		/* CAN-BUSのみ */
#define	HARD_WIRED_AND_CAN_BUS	2		/* ハードワイヤ、CAN-BUS 両方接続 */
#define	NOT_CONNECT		3		/* ハードワイヤ、CAN-BUS 両方未接続 */

// Lite 環境
#define	GSNS_BUILTIN		TRUE		/* Gセンサの有無 */

#define	TSNS_BUILTIN		TRUE		/* 温度センサの有無 */
#define	CAN_WINKER_INFO		FALSE		/* CAN-BUSからのウィンカー情報の有無 */
#define	WEAVING_DETECT		FALSE		/* ふらつき検知 */

#define	PRODUCT_DEST		AFTER		/* 仕向け */
#define	INSTALL_GRADE		LEVEL		/* 取付け角度 */
#define	DEFAULT_MOVE		FALSE		/* 未学習でも走行を開始するか？ */
#define	FORWARD_LOGIC		REV_UNKNOWN	/* 前進論理 */

#define	ADC_TYPE		ADS7828		/* A2Dコンバータの種類 */
#define	GYRO_TYPE		XV_8100CB	/* ジャイロの種類 */
#define	GSNS_TYPE		LIS244ALH	/* G-Sensorの種類 */
#define	TSNS_TYPE		S_58LM20A	/* 温度センサの種類 */

#define	TYRE_CHANGE		AUTO_CORR	/* タイヤ外径変化検出 */
#define	SETTING_CHANGE		NON_CORR	/* 取付け角度変更検出 */
#define	GSNS_INSTALL		REVERSE_SET	/* Gセンサ取付け向き */

#define	F_SNS			CYCLE_5HZ	/* 位置更新周期 [Hz] */

#define	VSS_SOURCE		HARD_WIRED_ONLY	/* 車速パルス信号情報源 */
#define	REV_SOURCE		HARD_WIRED_ONLY	/* リバース信号情報源 */

/* 変更の可能性あり by kato */
#define	PULSE_FILTER		FALSE		/* パルスフィルタがPSW OUTに接続されているか？ */


////
/***  GYRO学習用メモリ番号コード  ***/
#define	GYRO_MEM_A		0x00		/* Ａ用メモリ使用 */
#define	GYRO_MEM_B		0x01		/* Ｂ用メモリ使用 */



/* 車速パルス関連 */
#define	HI_PULSE	0		/* 車速パルスフィルタモード 多パルス車 */
#define	LO_PULSE	1		/* 車速パルスフィルタモード 1～4パルス車 */
#define	NORMAL_CORRECT	0		/* 自動距離補正モード */

/*** ジャイロ関連 ***/
/* ジャイロオフセットに関する定義 */
#if (GYRO_TYPE == XV_8100CB)	/* エプソントヨコムの場合 */
#define	GYRO_OFFSET		1.35F		/* 0点電圧値 : 1.35[V] */
#define	GYOFF_DEF		(static_cast< FLOAT >(GYRO_OFFSET*4096.0F*25.0F/2.5F))
						/* ジャイロオフセット値[LSB/Block] */
#define	LIMIT_GYOFF		2048.0F		/* ジャイロの中点のとりうる値[LSB] */
						/* 1.35Vからの差分値 [LSB]  電圧換算50mV */
						/* 4096.0F/2.5F[V]/0.05F[V]*25 */
#else	/* それ以外のジャイロの場合 */
#define	GYRO_OFFSET		2.5F		/* 0点電圧値 : 2.5[V] */
#define	GYOFF_DEF		(static_cast< FLOAT >(GYRO_OFFSET*4096.0F*25.0F/5.0F))
						/* ジャイロオフセット値[LSB/Block] */
#define	LIMIT_GYOFF		6144.0F		/* ジャイロの中点のとりうる値[LSB] */
						/* 2.5Vからの差分値 [LSB]  電圧換算300mV */
#endif
/* ジャイロ感度に関する定義 */
#if (GYRO_TYPE == FAR_S1)	/* 02HDD-AVNに搭載の富士通ジャイロの場合 */
#define	GYRO_SENS		(0.025F*1.02F)	/* 感度 : 0.0255[V/deg/s] */
#elif (GYRO_TYPE == XV_8100CB)	/* エプソントヨコムの場合 */
#define	GYRO_SENS		0.0025F		/* 感度 : 0.0025[V/deg/s] */
#else 				/* その他のジャイロ */
#define	GYRO_SENS		0.025F		/* 感度 : 0.025[V/deg/s] */
#endif

#if (GYRO_TYPE == XV_8100CB)	/* エプソントヨコムの場合 */
#define	INITIAL_GYRO_GAIN_VALUE (static_cast< FLOAT >(2.5F/4096.0F/GYRO_SENS/500.0F))
						/* ジャイロゲイン値[deg/s/LSB/Block] */
#else 				/* その他のジャイロ */
#define	INITIAL_GYRO_GAIN_VALUE (static_cast< FLOAT >(5.0F/4096.0F/GYRO_SENS/500.0F))
						/* ジャイロゲイン値[deg/s/LSB/Block] */
#endif

#define	GAIN_DEF		(static_cast< FLOAT >(INITIAL_GYRO_GAIN_VALUE*INSTALL_GRADE))

#if (GYRO_TYPE == XV_8100CB)	/* エプソントヨコムの場合 */
#define	GYGAIN_MAX_VALUE	(INITIAL_GYRO_GAIN_VALUE*1.4F)	/* ゲインの最大値 : +40% */
								/* (傾き45度 30%+スペック1.25%+学習時のばらつき5%+余裕分) */
#define	GYGAIN_MIN_VALUE	(INITIAL_GYRO_GAIN_VALUE*0.9F)	/* ゲインの最小値 : -10% */
								/* (スペック1.25%+学習値のばらつき5%) */
#else 				/* その他のジャイロ */
#define	GYGAIN_MAX_VALUE	(INITIAL_GYRO_GAIN_VALUE*1.5F)	/* ゲインの最大値 : +50% */
								/* (傾き45度 30%+スペック10%+学習時のばらつき5%+余裕分) */
#define	GYGAIN_MIN_VALUE	(INITIAL_GYRO_GAIN_VALUE*0.85F)	/* ゲインの最小値 : -15% */
								/* (スペック10%+学習値のばらつき5%) */
#endif

/* ジャイロ停止判定レベルに関する定義 */
#define	GYRO_OFF_LEVEL		40.0F		/* 停止レベル初期値 */
#define	GYRO_SET_OK		GYRO_OFF_LEVEL	/* 停止レベル1 */
#define	GYRO_SET_MORE_OK	10.0F		/* 停止レベル2 */
#define	GYRO_SET_MOST_OK	5.0F		/* 停止レベル3 */

/*** Gセンサ停止判定レベルに関する定義 ***/
/* 三菱製 or 東プレ傾き製 or 松下電工製 or STMicro製の場合 */
#if (GSNS_TYPE == MAS1390P) || (GSNS_TYPE == TPR70_GC5) || (GSNS_TYPE == AGS19001) || (GSNS_TYPE == LIS244ALH)
#define	GSNS_OFF_LEVEL		80.0F		/* 初期値:80 (= 20.0*1.667*1.667+余裕) */
#define	GSNS_OFF_LEVEL_MAX	850.0F		/* 最大値:850 (= 300.0*1.667*1.667+余裕 */
#define	GSNS_SET_OK		420.0F		/* 停止レベル1 (= 150.0*1.667*1.667+余裕) */
#define	GSNS_SET_MORE_OK	GSNS_OFF_LEVEL	/* 停止レベル2 */
#define	GSNS_SET_MOST_OK	20.0F		/* 停止レベル3 (= 7.0*1.667*1.667+余裕) */
#else	/* 02モデル以前の東プレGセンサの場合 */
#define	GSNS_OFF_LEVEL		20.0F		/* 初期値:20 */
#define	GSNS_OFF_LEVEL_MAX	300.0F		/* 最大値:300 */
#define	GSNS_SET_OK		150.0F		/* 停止レベル1 */
#define	GSNS_SET_MORE_OK	GSNS_OFF_LEVEL	/* 停止レベル2 */
#define	GSNS_SET_MOST_OK	7.0F		/* 停止レベル3 */
#endif

/*** Gセンサ感度に関する定義 ***/
/* 三菱製 or 東プレ傾き製 or 松下電工製の場合 */
#if (GSNS_TYPE == MAS1390P) || (GSNS_TYPE == TPR70_GC5) || (GSNS_TYPE == AGS19001)
#define	GSNS_SENS		1.0F		/* 感度 : 1.0[V/G] */
#else	/* 02モデル以前の東プレGセンサ、それ以外のGセンサの場合 */
#define	GSNS_SENS		0.6F		/* 感度 : 0.6[V/G] */
#endif

#define	G_A			9.80665F	/* 重力加速度[m/s2] */

#if (GSNS_TYPE == LIS244ALH)	/* STMicroGセンサの場合 */
#define	G_25LSB			((FLOAT)(GSNS_SENS*4096.0*25.0/2.5))
						/* １ブロックのGセンサ感度[LSB/Block/G] */
#else	/* それ以外のGセンサの場合 */
#define	G_25LSB			((FLOAT)(GSNS_SENS*4096.0*25.0/5.0))
						/* １ブロックのGセンサ感度[LSB/Block/G] */
#endif

#define	G_GAIN			(G_A/G_25LSB)	/* Gセンサゲイン値[m/s2/LSB/Block] */

#define	GSNS_GAIN_DEF		INSTALL_GRADE	/* Gセンサゲインの初期値 */

/* Gセンサオフセットに関する定義 */
#if (GSNS_TYPE == MAS1390P) || (GSNS_TYPE == AGS19001) /* 三菱製or松下電工製の場合 */
#define	GSNS_OFFSET		2.5F		/* 0点電圧値 : 2.5[V] */
#elif (GSNS_TYPE == TPR70_GC5)	/* 03モデル東プレGセンサの場合 */
#define	GSNS_OFFSET		1.8F		/* 0点電圧値 : 1.8[V] */
#elif (GSNS_TYPE == LIS244ALH)	/* STMicroGセンサの場合 */
#define	GSNS_OFFSET		1.5F		/* 0点電圧値 : 1.5[V] */
#else				/* 02モデル以前の東プレGセンサの場合 */
#define	GSNS_OFFSET		2.0F		/* 0点電圧値 : 2.0[V] */
#endif

#if (GSNS_TYPE == LIS244ALH)	/* STMicroGセンサの場合 */
#define	GSNS_OFS_DEF		(static_cast< FLOAT >(GSNS_OFFSET*4096.0F*25.0F/2.5F))	/* Gセンサオフセット値[LSB/Block] */
#else				/* それ以外のGセンサの場合 */
#define	GSNS_OFS_DEF		(static_cast< FLOAT >(GSNS_OFFSET*4096.0F*25.0F/5.0F))	/* Gセンサオフセット値[LSB/Block] */
#endif

/* 温度センサ関連 */
#define	GAIN2WORD		(65535.0F/2.0F)	/* ゲイン値 - WORD変換 */
#define	TSNS_DEV_NUM		4		/* 温度を取得するデバイス数 */
#define	TSNS_GYRO_OFFSET	0		/* ジャイロオフセット */
#define	TSNS_GYRO_GAIN		1		/* ジャイロゲイン */
#define	TSNS_GSNS_OFFSET	2		/* Ｇセンサオフセット */
#define	TSNS_GSNS_GAIN		3		/* Ｇセンサゲイン */
#define	TEMPERATURE_ERROR_NUM	1000.0F		/* 温度異常値[℃] */



#define	EXCELLENT_DIM		3			/* エクセレント学習に使用する近似関数の次数 */ 
#define	ERR_INDEX_MAX		25.5F		/* 誤差指数最大値 */

/***  センサリセット状態返却コード  ***/
#define	SNS_RESET_OK		0x01		/* メモリリセット実行 */
#define	SNS_RESET_NG		0x00		/* メモリリセットしていない */

#define	GSNS_ANGLE		1		/* 3Dハイブリットで学習 */

/***  左右取付け角度用定数  ***/
#define	CENTER_ANGLE		0		/* センターの取付け角度 */
#define	RIGHT_ANGLE		1		/* 右向きの取付け角度 */
#define	LEFT_ANGLE		2		/* 左向きの取付け角度 */

#define	CHECK_OFF	0		/* 車速パルス関連バックアップチェックを行わない場合 */

#define	GPS_PITCH_NG		100.0F		/* GPSピッチ角を初期化する */


/*** sns_model.h ***/

/* モデル別の動作状態の構造体 */
typedef struct  {
    BYTE    GSnsBuiltin;		/* TRUE GSNSあり FALSE GSNSなし */
    BYTE    TSnsBuiltin;		/* TRUE TSNSあり FALSE TSNSなし */
    BYTE    CanWinkerInfo;		/* CAN-BUSからのウィンカー情報 TRUE あり FALSE なし */
    BYTE    WeavingDetect;		/* TRUE ふらつき検知あり FALSE ふらつき検知なし */
    BYTE    Destination;		/* 仕向け */
    FLOAT   InstallGrade;		/* 取り付け角度 */
    BYTE    DefaultMove;		/* 未学習でも走行を開始するか？ */
    BYTE    ForwardLogic;		/* 前進論理 */
    BYTE    AdcChip;			/* A2Dコンバータの種類 */
    BYTE    GyroDevice;			/* ジャイロの種類 */
    BYTE    GSnsDevice;			/* G-Sensorの種類 */
    BYTE    TSnsDevice;			/* 温度センサの種類 */
    BYTE    TyreChange;			/* タイヤ外径変化検出の補正方法 */
    BYTE    SettingChange;		/* 取付け角度変更検出の補正方法 */
    BYTE    GSnsInstall;		/* Gセンサの取付け向き */
    BYTE    VssSource;			/* 車速パルス信号情報源 */
    BYTE    RevSource;			/* リバース信号情報源 */
    BYTE    PulseFilter;		/* パルスフィルタがPSW OUTに接続されているか？ TRUE 接続 FALSE 非接続 */
} sSnsModelInfo;

/*** sns_bkup.h ***/

/*** センサバックアップデータ構造体宣言 ***/
/* センサ初期学習状態用構造体の宣言 */
typedef struct  {
    BYTE    SnsAllOk;		/* 全センサ使用可能フラグ */
    BYTE    ShopMode;		/* 店頭展示モード */
    BYTE    SpeedOk;		/* 距離学習値使用可能フラグ 0:学習値使用不可 1:学習値使用可能 */
    BYTE    GyroOk;		/* ジャイロ使用可能フラグ */
    BYTE    GSnsStatus;		/* Gセンサ使用可能フラグ */
    BYTE    RevLineOk;		/* バック線使用可能フラグ */
    BYTE    YawSetupValue;	/* Yaw方向の取付け角度設定値 */
} sSnsLearnStatus;

/* メールデータバックアップ変数構造体の宣言 */
typedef struct  {
    DWORD   PulseCounter;	/* パルスカウンタ */
    DOUBLE  Odometer;		/* オドメータ 1万kmでターンオーバー ユーザリセットあり */
    DOUBLE  ScheOdometer;	/* スケジューラ用オドメータ 100万kmでターンオーバー */
    FLOAT   DeltaLatSum;	/* メール非送信時の移動量保存 緯度方向 */
    FLOAT   DeltaLonSum;	/* 経度方向 */
    FLOAT   DeltaDSum;		/* 走行距離 */
    BYTE    DeltaSumOn;		/* 移動量保存済みフラグ */
    DWORD   CanBusEndOfLineSetting;	/* CAN-BUSからのEOL(end-of-line)設定情報 バックアップ変数 */
} sMailBackupData;

/* 車速パルスバックアップ変数構造体 基本部分の宣言 */
typedef struct  {
    FLOAT   PeriodLength;	/* 補正後の距離学習値[m] */
    FLOAT   PeriodLengthGps;	/* GPS補正後の距離学習値[m] */
    WORD    PeriodLengthCnt;	/* 距離学習回数カウンタ 0:未学習 1:分周のみ終了 2～:学習中 */
    WORD    PrdLenCorrMode;	/* 1周期の距離を補正するモード MANUAL,NOMAL,FAST*/
    CHAR    PeriodDivision;	/* パルスの分周数(1, 2, 4, 8分周が使用可能) */
    BYTE    PlsLineConnect;	/* FALSE:車速パルス未接続,  TRUE:車速パルスあり*/
    FLOAT   ManualCorr;		/* MANUAL距離補正係数 */
    BYTE    RevLineConnect;	/* リバース線が接続されているかを示すフラグ */
    BYTE    RevLogicSet;	/* リバース線の論理がセットされているかを示すフラグ */
    BYTE    ForwardLogic;	/* リバース線の前進論理 */
} sSpeedBackupBasic;

/* 車速パルスバックアップ変数構造体 拡張部分の宣言 */
typedef struct  {
    FLOAT   MapMatchCorr;	/* MM距離補正係数 */
    WORD    MapMatchCorrCnt;	/* MM distance correction counter */
    BYTE    PulseErr;		/* 車速パルスエラーフラグ TRUE:エラー有り FALSE:エラー無し */
    BYTE    PulseMode;		/* speed pulse filter mode */
    BYTE    PeriodLengthErr;	/* 車速パルス関連バックアップデータ状態フラグ TRUE:異常 FALSE:正常 */
    BYTE    LowSpeedNg;		/* 低速車速パルス未発生車両判断フラグ */
    WORD    MinimumSpeedCnt;	/* 最低出力速度学習回数 */
    FLOAT   MinimumSpeed;	/* 最低出力速度学習値 [m/sec] */
    FLOAT   LastPrdLen;		/* タイヤ外径変化検出 前回補正前距離学習値 */
    WORD    LastPrdLenCnt;	/* 前回補正前距離学習回数 */
    SHORT   TyreErrCnt;		/* 検出判断用カウンタ */
    FLOAT   PhantomPrdLen;	/* 裏学習値[m] */
    BYTE    PulseSignalError;	/* 車速パルス異常 */
    BYTE    PastDivNum;		/* 過去分周比(JIS基準、分周比異常車両用)
					   |= 0x02 : 2パルス車相当
					   |= 0x04 : 4パルス車相当
					   |= 0x08 : 8パルス車相当 */
} sSpeedBackupExtension;

/* ジャイロセンサバックアップ変数構造体 基本部分の宣言 */
typedef struct  {
    FLOAT   Direction;		/* ジャイロセンサ方位データ */
    FLOAT   RightGain;		/* 右ゲイン */
    FLOAT   LeftGain;		/* 左ゲイン */
    FLOAT   CommonGain;		/* 左右共通ゲイン */
    BYTE    RGainCnt;		/* 右ゲインの学習度 */
    BYTE    LGainCnt;		/* 左ゲインの学習度 */
    BYTE    RGainCorrCnt;	/* 右ゲインの補正回数 */
    BYTE    LGainCorrCnt;	/* 左ゲインの補正回数 */
}sGyroBackupBasic;

/* ジャイロセンサバックアップ変数構造体 拡張部分の宣言 */
typedef struct  {
    BYTE    ExcellentQuality;	/* エクセレント補正関数のクオリティ 0:関数なし */
    SHORT   ExcellentWeek;	/* エクセレントを学習した週 */
    DWORD   ExcellentLimit;	/* エクセレント補正の終了時間 */
    FLOAT   ExcellentFunction[4];	/* エクセレント補正の回帰曲線の係数 3次関数に対応 */
    FLOAT   ExcellentAve;	/* エクセレント学習をしたときのオフセットの平均 */
    FLOAT   Offset;		/* オフセット値 */
    FLOAT   DirErr;		/* 方位誤差 */
    FLOAT   DirErrRelief;	/* 方位誤差(リリーフ動作用) */
    FLOAT   SigmaLevel;		/* Gyro 停止判定のシグマの閾値 */
    FLOAT   GainMovingAve[2];	/* 最新ジャイロゲイン生値平均値 */
    BYTE    SettingChangeCnt[2];	/* 設置角度変更検出カウンター */
    BYTE    SensitivityErrCnt;	/* 感度異常検出用カウンタ */
    BYTE    GyroDeviceError;	/* Gyroセンサ異常 */
    BYTE    AdcDeviceError;	/* A/Dコンバータ異常 */
    BYTE    TsnsDeviceError;	/* 温度センサ異常 */
}sGyroBackupExtension;

/* Gセンサバックアップ変数構造体 基本部分の宣言 */
typedef struct  {
    BYTE    GSnsErr;		/* 設置方向判定フラグ(1: NG) */
    BYTE    SettingDir;		/* 前後方向判定フラグ(0:不定,1: 反転,2: 正面) */
    BYTE    DistanceSet;	/* 距離使用可能判断フラグ (TRUE: set) */
    FLOAT   Offset;		/* オフセット値*/
    WORD    OffsetCnt;		/* オフセットカウンタ */
    BYTE    SettingSide;	/* 回転方向の取り付け向き(右向き:1, 左向き:2) */
    FLOAT   YawTheta;		/* Yaw方向の取り付け角度 */
    FLOAT   IncGain;		/* 加速方向のゲイン補正値 */
    FLOAT   DecGain;		/* 減速方向のゲイン補正値 */
    WORD    IncGainCnt;		/* 加速方向のゲイン補正カウンタ */
    WORD    DecGainCnt;		/* 減速方向のゲイン補正カウンタ */
}sGSnsBackupBasic;

/* Gセンサバックアップ変数構造体 拡張部分の宣言 */
typedef struct  {
    FLOAT   SigmaLevel;		/* G-Sensor 停止判定のシグマの閾値 */
    FLOAT   SlopeAngle;		/* 傾斜角 ( down:[-], up:[+] ) */
    WORD    YawThetaCnt;	/* Yaw方向の取り付け角度学習回数 */
    BYTE    SettingSideCnt;	/* 回転方向の取り付け向き学習カウンタ */
    FLOAT   CommonGain;		/* 加減速共通ゲイン */
    FLOAT   InstallPitchAngle;	/* ピッチ方向取り付け角度 */
    BYTE    Reserve;		/* Reserve */
}sGSnsBackupExtension;

/* 温度センサバックアップ変数構造体 */
typedef struct  {
    BYTE    GyroOfsAve[TSNS_OFFSET_MAX];	/* ジャイロオフセット平均化カウンタ */
    BYTE    GyroGainAve[TSNS_GAIN_MAX];		/* ジャイロゲイン平均化カウンタ */
    BYTE    GSnsOfsAve[TSNS_OFFSET_MAX];	/* Gセンサオフセット平均化カウンタ */
    BYTE    GSnsGainAve[TSNS_GAIN_MAX];		/* Gセンサゲイン平均化カウンタ */
    SHORT   GyroOffset[TSNS_OFFSET_MAX];	/* ジャイロオフセット */
    WORD    GyroGain[TSNS_GAIN_MAX];		/* ジャイロゲイン */
    SHORT   GSnsOffset[TSNS_OFFSET_MAX];	/* Gセンサオフセット */
    WORD    GSnsGain[TSNS_GAIN_MAX];		/* Gセンサゲイン */
} sTSnsBackupData;


/* センサバックアップ変数構造体 基本部分の宣言 */
typedef struct  {
    sSpeedBackupBasic		Speed;
    sGyroBackupBasic		Gyro;
    sGSnsBackupBasic		GSns;
} sSensorBackupBasic;

/* センサバックアップ変数構造体 拡張部分の宣言 */
typedef struct  {
    sSpeedBackupExtension	Speed;
    sGyroBackupExtension	Gyro;
    sGSnsBackupExtension	GSns;
} sSensorBackupExtension;

/* 傾斜旋回補正誤差見積もり */
typedef struct  {
    FLOAT   DegreeOfSnsDirErrorPosi;		/* センサ方位誤差積算[DEG] */
    FLOAT   DegreeOfSnsDirErrorPosiAbs;		/* センサ方位誤差積算(絶対値)[DEG] */
    FLOAT   DegreeOfSnsDirErrorNega;		/* センサ方位誤差積算[DEG] */
    FLOAT   DegreeOfSnsDirErrorNegaAbs;		/* センサ方位誤差積算(絶対値)[DEG] */
    /* 取付け角度推測及び傾斜ゲイン補正用 */
    FLOAT   SetupAngleDeg;			/* 推測取付け角度の絶対値 単位[度] */
} sSensorBackupSetupIncEstErr;

/* CanBus車速パルス異常検出用構造体の宣言 */
typedef struct  {
    BYTE    DivNumErrLevel;	/* 分周比異常レベル(JIS基準) */
    BYTE    NonlinearLevel;	/* 非線形性レベル */
    BYTE    StepwiseLevel;	/* 階段状レベル */
    BYTE    AlternusLevel;	/* 交互変化レベル */
    BYTE    Reserve1;		/* Reserve */
    BYTE    Reserve2;		/* Reserve */
    BYTE    Reserve3;		/* Reserve */
    BYTE    Reserve4;		/* Reserve */
} sCanBusPulseErr;

/* 速度に対する距離学習値保存用構造体の宣言 */
typedef struct  {
    FLOAT   PrdLength;		/* 速度に対する距離学習値 */
    FLOAT   SpeedAve;		/* 距離学習時の平均速度 */
    WORD    PrdLengthCnt;	/* 各速度の距離学習回数 */
} sPeriodLengthExtension;

/* 距離学習値近似曲線用構造体の宣言 */
typedef struct  {
    FLOAT   PrdLengthCorr[PRD_LEN_CORR_BUF_SIZE];	/* 距離学習値近似曲線係数格納バッファ */
    BYTE    PrdLengthProcess;	/* 距離学習値近似曲線算出ステータス */
    BYTE    UseBufCnt;		/* 距離学習値近似曲線算出使用バッファ数 */
    BYTE    PrdLengthOk;	/* 距離学習値近似曲線使用可能フラグ */
} sApproxPeriodLengthData;

/***  センサバックアップデータの状態設定用定数  ***/
enum NPSnsSnsMemEnum{
    NP_SNS_MEM_CONDITION_NG = 0,	// センサメモリ 利用不可
    NP_SNS_MEM_CONDITION_OK = 1,	// センサメモリ 利用可能(学習値有り)
    NP_SNS_MEM_CONDITION_MISSING = 2	// センサメモリ 復帰処理失敗(学習値無し)
					// センサメモリ復帰処理をおこなったが復帰できなかった場合
} ;

/*** bios_gps.h ***/

/* GPS位置構造体 */
typedef	struct {
    WORD    StatusCode;		/* 測位ステータス */
    WORD    TrackSat;		/* 測位使用可能衛星数 */
    DWORD   SvNum;		/* 捕捉衛星番号 */
    LONG    Latitude;		/* 緯度 */
    LONG    Longitude;		/* 経度 */
    SHORT   Altitude;		/* 高度 */
    SHORT   Velocity;		/* 水平方向の速度 */
    SHORT   Heading;		/* 水平方向速度の方位 */
    SHORT   Climb;		/* 垂直方向の速度 */
    BYTE    Hdop;		/* HDOP水平方向精度 */
    BYTE    Vdop;		/* VDOP高度方向精度 */
    WORD    Laxis;		/* 誤差楕円長軸 */
    WORD    Saxis;		/* 誤差楕円短軸 */
    WORD    LaDir;		/* 誤差長軸の傾き */
    BYTE    VelAcc;		/* 速度の精度 */
    BYTE    DirAcc;		/* 方位の精度 */
    BYTE    LocalTime[6];	/* 現在時刻データ */
    BYTE    FixDelay;		/* 測位時刻からの遅れ */
} sGpsPosData;

/* 各GPS衛星の情報 */
typedef struct {
    BYTE    SvPrn;		/* 衛星番号 */
    BYTE    SvStatus;		/* 衛星の捕捉状態 */
    BYTE    Elevation;		/* 仰角 */
    BYTE    Azimuth;		/* 方位角 */
    SHORT   Doppler;		/* ドップラー周波数 */
    SHORT   SearchRange;	/* サーチ幅 */
    BYTE    SvLevel;		/* 受信レベル (AMU) */
} sGpsSatData;

/* GPS衛星情報構造体Type2 */
typedef	struct {
    sGpsSatData    CurrSat[12];		/* 12衛星分の衛星情報 */
    BYTE    TrackSat;			/* 捕捉衛星数 */
    BYTE    Reserve[3];			/* リザーブ */
} sGpsSatStatus2;

/*** gps_bios.h ***/

/*****************************************************/
/* GPS測位結果を格納する構造体                       */
/*  1. センサプログラム内で使用するGPSデータ         */
/*  2. LOCタスクに渡すデータの元データとして使用する */
/*****************************************************/
typedef	struct  {
    sGpsPosData		GpsPosDataBuf;	/* GPS位置構造体 */
    sGpsSatStatus2	GpsSatDataBuf;	/* GPS衛星情報構造体 */
    DOUBLE  curr_pos_ecef[4];	/* 現在位置 (ECEF座標) */
    FLOAT   AdjPdop;		/* 位置の精度劣化係数 (2D時は最終3DのVDOPを使用して計算）*/
    FLOAT   hdop;		/* 水平方向の精度劣化係数 */
    FLOAT   vdop;		/* 垂直方向の精度劣化係数 */
    FLOAT   Heading;		/* 方位 */ 
    FLOAT   Velocity;		/* 2次元速度 [m/s] */
    FLOAT   Climb;		/* 垂直方向の速度 */
    DOUBLE  FixTime;		/* センサで使用する最終測位時刻 */
    SHORT   Laxis;		/* 誤差長軸半径 */
    SHORT   Saxis;		/* 誤差短軸半径 */
    SHORT   Angle;		/* 誤差長軸傾き */
    BYTE    DgpsNow;		/* 0:GPS, 1:DGPS */
    BYTE    TS;			/* 捕捉衛星数 */
    BYTE    GpsDim;		/* 測位次元 [2: 2D測位, 3: 3D測位] */
    BYTE    CHMFix;		/* 0:通常測位, 1:Clock Hold Mode */
    BYTE    posfix_flag;	/* get position flag */
    BYTE    status_code;	/* system status code number */
    BYTE    FixUpdate;		/* 測位データアップデートフラグ （センサタスク専用）*/
				/* 1: 測位データアップデート */
				/* 2: 測位データ計算中 (GPSタスク動作中) */
				/* 0: 測位データはセンサタスクで使用済み */
    BYTE    MaxAMU;		/* 捕捉中の衛星の最大ＡＭＵ値 */
    BYTE    MaxAMUSv;		/* 最大ＡＭＵ値の衛星番号 */
    BYTE    SnsGpsStatus;	/* センサ利用GPSの測位ステータス (コピー) */
    BYTE    GpsSigStatus;	/* GPSの情報ステータス */
				/*   bit0  : 1=衛星の信号が全く受信できない */
				/*   bit1-2: マルチパス検出レベル           */
				/*   bit3  : 1=SAあり, 0=SAなし             */
				/*   bit4-7: 予備                           */
    DWORD   GpsMultiStatus;	/* GPSのデバッグ情報ステータス */
				/*  デバッグ用の汎用ステータス情報 */
				/*  （ビットの意味が変更される場合があります） */
} sGpsSendData;

/* センサプログラムで使用するGPSデータ構造体 */
typedef struct  {
    sGpsSendData *Mail;			/* GPS測位結果を格納する構造体 */
    FLOAT   Velocity3D;			/* 3次元速度 */
    FLOAT   PitchAngle;			/* GPS速度のピッチ角 */
    FLOAT   LastVelocity;		/* GPSの前回速度 */
    FLOAT   LastHeading;		/* GPSの前回方位 */
    FLOAT   LastFixTime;		/* GPSの前回測位時刻 */
    FLOAT   DelayTime;			/* GPSの測位時間からのディレイ sdp->t_meas_gyro - fix_time[sec] */
    BYTE    FixContinue;		/* GPS連続測位判定 */
} sGpsInSns;

/* sns_handle関連 */
#define	SPPRD_BUF	5		/* パルス周期格納バッファの数(CPLD) */
#define	GYRO_CH		0		/* A/Dコンバータのチャンネル ジャイロ (AIN1) */
#define	GSNS_CH		1		/* A/Dコンバータのチャンネル Gセンサ (AIN2) */
#define	TSNS_CH		2		/* A/Dコンバータのチャンネル 温度センサ (AIN4) */
#define	NUM_AD_SUM	3		/* 積算するA/Dチャンネル数 GYRO,GSNS,TSNS */

/*** sns_gyro.h ***/
/* システム関連 */
#define	SNS_BLOCK	50			/* 1ブロックの長さ 50[msec] */
#define	SNS_BLOCK_SEC	0.05F			/* 1ブロックの長さ 0.05[sec]*/
#define	F_BLOCK		(1000/SNS_BLOCK)	/* 1秒間に処理するブロックデータ 20[個] or [Hz] */
#define	SNS_BUF		(F_BLOCK/F_SNS)		/* 1生データで処理するブロック数 */
#define	SNS_BUF_MSEC	(SNS_BUF*SNS_BLOCK)	/* 1生データの長さ 50 * SNS_BUF[msec] */
#define	SNS_BUF_SEC	(SNS_BUF_MSEC/1000.0F)	/* 1生データの長さ 0.05 * SNS_BUF[sec] */

#define	CYCLE_5HZ		5		/* 5Hz動作 */

/* センサのブロック生データ (50ms) */
typedef struct  {
    LONG    AdSum[NUM_AD_SUM];		/* 1ブロック間のA/Dの積算値 GYRO, GSNS, TSNS */
    SHORT   SpPulseCnt;			/* 1ブロック中のパルス数 */
    DWORD   SpPeriod[SPPRD_BUF];	/* パルス周期 */
    BYTE    SpPeriodCnt;		/* パルス周期数 */
    BYTE    ReverseSense;		/* リバースギア端子の状態 1:バック信号 High, 0:バック信号 Low */
    CHAR    AdjustMsec;			/* Adjust Msec(intmsをずらした時:測位中)が発生したこと示すフラグ */
					/* SnsMsecCounterの初期値を代入する -1 or 1 */
} sSnsBlock;

/* センサ生データ 構造体 */
typedef struct  {
    sSnsBlock BlockData[SNS_BUF];	/* ブロック生データ */
    DOUBLE  TimeMeasure;		/* センサ生値取得時刻 [time of week] */
    DWORD   SnsSec;			/* センサ生値取得時刻 [sec] */
    WORD    SnsMsec;			/* センサ生値取得時刻 [msec] */
    CHAR    PeriodDivision;		/* パルスの分周数(1, 2, 4, 8分周が使用可能) */
					/*  -1: 分周変更あり, 1: 分周無し */
					/*   2: 2分周, 4: 4分周, 8: 8分周 */
    WORD    PulseCntSum;		/* 生データ中の全パルス数 */
    BYTE    PulseRemainder;		/* 車速パルスの片エッジのみ検出したことを示すフラグ */
    BYTE    PulseErrCnt;		/* パルス数エラーのカウンタ */
    BYTE    AdErrCnt[NUM_AD_SUM];	/* A/Dエラーの回数 0～SNS_BUF */
    FLOAT   AdAve[NUM_AD_SUM];		/* A/D値の平均 */
    FLOAT   AdSigma[NUM_AD_SUM];	/* A/D値のシグマの値 */
    DWORD   SnsTimer;			/* 電源ONからの生データカウンタ */
    WORD    MinSpPlsLen[2];		/* 停止、発進時における最低速度のパルス幅[msec] */
    FLOAT   Temperature;		/* 温度センサの温度 */
    BYTE    Winker;			/* ウインカー信号(左、右、無しをBitで表現して使用する事) */
} sSnsRawData;

/* センサ移動距離、角度変化量の積算処理 */
typedef	struct  {
    FLOAT   DeltaD;			/* 1生データ(SNS_BUF * 50ms)間の水平方向の走行距離[m] */
    FLOAT   DeltaX;			/* 1生データ間の経度方向走行距離[m] */
    FLOAT   DeltaY;			/* 1生データ間の緯度方向走行距離[m] */
    FLOAT   DeltaZ;			/* 1生データ(SNS_BUF * 50ms)間の垂直方向の走行距離[m] （未使用） */
    FLOAT   DeltaTheta;			/* 1生データ(SNS_BUF * 50ms)間の角度変化量 */
    FLOAT   AngularVel;			/* 角速度 [deg/sec] (時計回り:正) */
} sSnsDelta;

#define	SNS_BUF_CNT	60			/* 生データを格納するバッファの数 */
						/* センサドライバでの確保分(5秒) + マージン(1秒分) */

/* センサ学習データ復帰前エラー情報 */
typedef	struct  {
    BYTE    ErrNum;	/* エラー番号 */
    DWORD   Value;	/* 任意の値 */
    DWORD   Lines;	/* エラーが発生した行番号 */
    CHAR    Id[82];	/* VERSIONで定義したRCS ID文字列へのポインタ */
} sStoreErrInfo;

class NPSnsHandlerBase
{

    /*** sns_model.h ***/

    SNS_GLOBAL	sSnsModelInfo	SnsModelInfo;		/* モデル別の動作状態 */

    /*** sns_bkup.h ***/

    /***** センサバックアップデータ ****/
    /*** 注意事項1 ***/
    /* センサバックアップデータは、自動的にメモリに配置されるため */
    /* SNS_SIGNWORDが、どのアドレスに配置されるかわからない       */
    /* SNS_SIGNWORDが、必ず、8BYTEアラインが入った後のアドレスに  */
    /* 配置されるようにするために調整用の変数の宣言を行う         */
    /* この変数以降にセンサバックアップ変数の追加を行う事         */

    /*** 注意事項2 ***/
    /* BackupCheckSumまでは固定の配置とするため変更しないこと     */

    /*** 注意事項3 ***/
    /* 変数の追加を行った場合には、SRAM管理番号を定義しなおすこと */
    /* 同時にバックアップバージョンの定義をしなおすこと           */

    /*** 注意事項4 ***/
    /* センサバックアップデータの合計サイズは、1024byteまでとする */
    /* そのため、変数の追加を行った場合には、サイズの確認を行う事 */
    /* 1024byteを超える場合には、仕組みの再編を行う必要あり       */
    /* Version1時点で、845+3Byte利用                              */

    SNS_GLOBAL	DOUBLE	bkAdjustmentData;		/* 調整用変数（実バックアップ変数ではない） */

    /* センサバックアップデータに変数を追加する場合には、以下の変更が必要 */
    /* センサ生値バージョンの変更、SRAM管理番号の変更、SIMULATORの対応 */
    /* ここから センサバックアップデータ */
    SNS_GLOBAL_B	WORD	SNS_SIGNWORD;		/* SENSOR SIGNWORD */
    SNS_GLOBAL_B	DWORD	SnsSRamNow;		/* 現在のSNS-SRAM管理番号 */
    SNS_GLOBAL_B	BYTE	SnsMemory;		/* 現在使用中のセンサメモリGYRO_MEM_A or GYRO_MEM_B */
    SNS_GLOBAL_B	BYTE	SnsBackupError;		/* バックアップデータエラーフラグ */
    SNS_GLOBAL_B	BYTE	BackupCheckSum;		/* バックアップデータのチェックサム */

    /* センサ初期学習状態用構造体 */
    SNS_GLOBAL_B	sSnsLearnStatus		SnsLearnStatus;

    /* センサバックアップ変数構造体 基本部分 */
    SNS_GLOBAL_B	sSensorBackupBasic	SensorBackupBasic;

    /*** GPS関連バックアップ変数 ***/
    /* 緯度、経度、高度 */
    SNS_GLOBAL_B	DOUBLE	bkCurrPosLla[3];
    SNS_GLOBAL_B	WORD	bkWeekNumber;		/* 週番号 */
    SNS_GLOBAL_B	DWORD	bkMsecOfWeek;		/* 週時間 */

    /* 温度センサバックアップ変数構造体 */
    SNS_GLOBAL_B	sTSnsBackupData		TSnsBackupData;

    /* センサバックアップ変数構造体 拡張部分 */
    SNS_GLOBAL_B	sSensorBackupExtension	SensorBackupExtension;

    /* メールデータバックアップ変数構造体 */
    SNS_GLOBAL_B	sMailBackupData		SnsBkM;
    
    /* 傾斜旋回補正の誤差見積もり関連構造体 */
    SNS_GLOBAL_B	sSensorBackupSetupIncEstErr	SnsBkIncEstErr;

    /* CanBus車速パルス異常検出用構造体 */
    SNS_GLOBAL_B	sCanBusPulseErr		CanPulseErr;

    /* 速度に対する距離学習値保存用構造体 */
    SNS_GLOBAL_B	sPeriodLengthExtension	PeriodLengthEx[PRD_LEN_EX_BUF_SIZE];

    /* 距離学習値近似曲線用構造体 */
    SNS_GLOBAL_B	sApproxPeriodLengthData	ApxPeriodLengthData;

    SNS_GLOBAL_B	CHAR	sns_bat_sign;		/* signature for battery RAM */
    /* ここまで センサバックアップデータ */
    /* ここからは、センサバックアップデータの次に宣言する必要がある変数 */

    /* MID_AVNでは、SRAMへのbyte書き込みが出来ない(write線が無い)       */
    /* そのため、SRAMへの書き込み用のデータサイズを調整する必要がある   */
    /* データサイズを2byte倍数で調整する必要がある                      */
    /* 更にmemcpyでのコピーを考慮し4Byte倍数へ調整を行うものとする      */
    SNS_GLOBAL_B	CHAR	bkAdjustmentBuff[3];	/* センサバックアップデータサイズ調整用バッファ */
    /* ここまで、センサバックアップデータの次に宣言する必要がある変数 */

    /* sns_main.c関連 */
    SNS_GLOBAL	sSnsLearnStatus		*SnsBkL;	/* センサ初期学習状態用構造体へのポインタ */
    SNS_GLOBAL	sSensorBackupBasic	*SnsBkB;	/* センサバックアップ変数構造体 基本部分へのポインタ */
    SNS_GLOBAL	sSensorBackupExtension	*SnsBkE;	/* センサバックアップ変数構造体 拡張部分へのポインタ */

    /* sns_main.c関連 */
    SNS_GLOBAL	sSnsDelta	SnsDelta;	/* センサ移動距離、角度変化量の積算処理用 */

    /*** NPSnsHandlerBase.cpp ***/
    SNS_GLOBAL	sStoreErrInfo	StoreErrInfo[ERR_INFO_NUM];	/* センサ学習データ復帰前エラー情報用バッファ */


    /*** gps_bkup.h ***/

    /**********************************************
    *	Geodetic System
    */

    GPS_GLOBAL_B	SHORT	bkGeodPrm;	/* Datum No. */
    GPS_GLOBAL_B	DOUBLE	bkLocalAxis;	/* 準拠楕円体の赤道半径*/
    GPS_GLOBAL_B	DOUBLE	bkLocalFlat;	/* 準拠楕円体の赤道半径*/
    GPS_GLOBAL_B	DOUBLE	bkDeltaA;	/* 準拠楕円体の赤道半径WGS-84系との差)	*/
    GPS_GLOBAL_B	DOUBLE	bkDeltaF;	/* 準拠楕円体の赤道半径WGS-84系との差)	*/
    GPS_GLOBAL_B	DOUBLE	bkEcc2;		/* f(2-f) */
    GPS_GLOBAL_B	DOUBLE	bkOneMe2;	/* 1-bkEcc2 */
    GPS_GLOBAL_B	DOUBLE	bkDeltaX;	/* 原点平行移動量 */
    GPS_GLOBAL_B	DOUBLE	bkDeltaY;	/* 原点平行移動量 */
    GPS_GLOBAL_B	DOUBLE	bkDeltaZ;	/* 原点平行移動量 */

    /*** pos_mail.cpp ***/

    DOUBLE	M2SecLat;		/* [m]->[sec]への変換パラメータ */
    DOUBLE	M2SecLon;		/* [m]->[sec]への変換パラメータ */
    DOUBLE	M2SecRefLat;		/* 変換パラメータを計算した緯度 [RAD] */
    BYTE	GpsLastPosOk;		/* 有効な測位データがある */
    DOUBLE	LastMailAltitude;	/* メールデータに最後にセットしたGPS高度[m] */
    BYTE	MailDebugCnt;		/* 送信データのカウンタ 0-255 */
    BYTE	CautionStatusCnt;	/* GPS、センサ系エラー情報用カウンタ */
    BYTE	GpsCautionStatus;	/* GPS系エラー情報用(0:エラーなし 1:エラーあり) */

protected:
	    NPGpsStoreDataStatusStr *m_psDataStatus;            ///< API用共有メモリのポインタ
    sGpsSnsMail	GpsSnsMailBuff;			/* メールデータ構造体('00) */
    WORD	LatestWeekNumber;	// 週番号
    DWORD	LatestMsecOfWeek;	// 週時間

    NP_BOOL	NotifiedOfGpsFlag;		/* GPS測位データの通知がされていることを示すフラグ */
    NP_BOOL	ErrorInfoCondition;		/* エラー履歴復帰状態フラグ */
    BYTE	StoreErrInfoNum;		/* センサ学習データエラー情報保存用ポインタ */

	    NP_BOOL	MakeErrorInfoFlag;	// エラー履歴更新フラグ
    NPSnsSnsMemEnum	SnsMemCondition;		/* センサバックアップデータ状態 */

    /*** sns_main.cpp ***/
    sGpsInSns		GpsInSns;		/* センサプログラムで使用するGPSデータ構造体 */
    FLOAT		TmpGpsLastVelocity;	/* GPS前回速度を作成するための変数 */
    FLOAT		TmpGpsLastHeading;	/* GPS前回方位を作成するための変数 */
    FLOAT		TmpGpsLastFixTime;	/* GPS前回測位時刻を作成するための変数 */

    GPS_GLOBAL	sGpsSendData	GpsMailData;	/* LOCへのメール構造体 */
    GPS_GLOBAL	BYTE		GpsTestMode;	/* GPS Test Mode On flag */
    GPS_GLOBAL	BYTE		SimLog;		/* GPS/SNS Data output for Simulation */

    sSnsRawData	SnsRawData[SNS_BUF_CNT];	/* 生データを格納する構造体 */
    BYTE	RawDataReadPtr;			/* SnsRawData構造体リードポインタ */
    BYTE	RawDataWritePtr;		/* SnsRawData構造体ライトポインタ */
    DWORD	LastSnsTimer;			/* 前回のセンサタイマ */

	    WORD	AdjSnsMemSize;	/* 4BYTE倍数へ調整済みセンサバックアップデータサイズ */
				/* アクセス用サイズなので実際のデータサイズとは異なるので注意 */
    /*** sns_decode.cpp ***/
    NP_BOOL		HaveGpsData;		/* Gpsデータ受信済みである事をフラグに示す */


	    SNS_GLOBAL	FLOAT	DeltaTheta;		/* 1生データ(SNS_BUF * 50ms)間の角度変化量 */

	GPS_GLOBAL	BYTE	Have1stFix;		/*  1: have had first good fix */

public:
	NPSnsHandlerBase();
	VOID	CopySnsBkSRAM(BYTE sram_num);
	BYTE	CalcSnsBackupCheckSum(VOID);
	BYTE	CheckSnsBackupSum(VOID);
	BYTE	VerifySensorBackupData(VOID);
	NP_BOOL	ReadToASectionSnsBkSRAM(VOID);
	NP_BOOL ReadToBSectionSnsBkSRAM(VOID);
	VOID	InitSensorBackupAll(VOID);
	VOID	MakeErrorInfo2(BYTE e_num, DWORD value, DWORD lines, const CHAR *id );
	VOID	SetSnsModelInfo(VOID);
	VOID	InitSensorBackupMailData(VOID);
	VOID	InitCanBusBkPrms(VOID);
	VOID	InitSensorBackup(VOID);
	VOID	InitGpsBackup(VOID);
	VOID	InitTSnsBackupData(VOID);
	VOID	InitIncEstErrorParameters(VOID);
	VOID	InitCanPulseErr(VOID);
	VOID	InitPeriodLengthEx(VOID);
	VOID	InitApxPeriodLengthData(VOID);
	VOID	WriteSensorBackupSignature(VOID);
    VOID	AddSnsBackupCheckSum(NP_BOOL req_to_send);
	VOID	GetCurTime(BYTE *buf);
	VOID	WriteToErrInfoSRAM(VOID);
	VOID	StoreErrorInfo(BYTE	e_num, DWORD value,	DWORD lines, const CHAR	*id);

	VOID	InitSensorBackupBasic(sSensorBackupBasic *bk_b);
	VOID	InitSensorBackupExtension(sSensorBackupExtension *bk_e);
	VOID	InitSnsLearnStatus(sSnsLearnStatus *bk_l);
	VOID	InitSpeedBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	);
	VOID	InitGyroBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	);
	VOID	InitGSnsBackupBasic(
	sSensorBackupBasic *bk_b  /* バックアップ構造体 基本部分へのポインタ */
	);
	VOID	WriteToSnsBkSRAM( VOID );
	VOID	InitSpeedBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	);
	VOID	InitGyroBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	);
	VOID	InitGSnsBackupExtension(
	sSensorBackupExtension *bk_e  /* バックアップ構造体 拡張部分へのポインタ */
	);
	VOID	InitSpeedAllParameters(VOID);
	VOID	InitSpeedPrms(
									 BYTE code
									 );
    VOID InitDirectPrms(
	LONG	elapsed_time,	/* 前回電源OFFからの経過時間 [秒] */
	SHORT	gps_week_num	/* 起動時のGPS週番号 */
	);
	VOID	InitGyroAllParameters(VOID);
	VOID	InitGSnsAllParameters(VOID);
	VOID	InitAccelPrms(VOID);
	VOID	InitTSnsAllParameters(VOID);
	VOID	InitSensorVariables(
	LONG elapsed_time,		/* 前回電源OFFからの経過時刻 ［秒］*/
	SHORT gps_week_num		/* 起動時のGPS週番号 */
	);

	VOID	InitSensor(VOID);
	VOID	InitSensorMain(VOID);
	VOID	MainBackupCheck(VOID);


	VOID	ReadToSnsBkSRAM(VOID);

	VOID	InitSensorDecodeVariable(VOID);
	VOID	ReadToErrInfoSRAM( VOID );

	VOID AddErrorInfo(VOID);
    VOID OutputTimeRETAILMSG(
	unsigned char	process_id,
	DWORD		value
	);

	VOID	ComputeS2M(
    DOUBLE	latitude	/* 緯度 [sec] */
    );

	VOID	OnInitialize(VOID);

};