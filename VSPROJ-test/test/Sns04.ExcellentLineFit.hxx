#include <Windows.h>

#include <iostream>
#include <cmath>

using namespace std;


#define	EXCELLENT_DIM		3			/* エクセレント学習に使用する近似関数の次数 */ 

#define	CYCLE_5HZ		5		/* 5Hz動作 */
#define	F_SNS			CYCLE_5HZ	/* 位置更新周期 [Hz] */
#define	SAMPLE_INTERVAL		(F_SNS*30)	/* オフセットをサンプルする間隔 */
#define	EXCELLENT_END1		(F_SNS*60*15)	/* 電源ON後この時間以上経過したらエクセレント学習を終了する */
#define	MAX_SAMPLE_EXCELLENT	(EXCELLENT_END1/SAMPLE_INTERVAL)	/* 最大サンプル数 */

#define	EXCELLENT_CNT_ERR	(F_SNS*5)	/* エクセレント学習の誤差がこの時間以上続いた場合、補正をOFFにする */
#define	EXCELLENT_STOP		(F_SNS-1)	/* エクセレント学習で使用する停止オフセットの平均化スレ */
#define	REG_NOT_CALC		0		/* エクセレント回帰曲線の計算ステップ 0 */
#define	REG_STEP1		1		/* エクセレント回帰曲線の計算ステップ 1 */
#define	REG_STEP2		2		/* エクセレント回帰曲線の計算ステップ 2 */
#define	REG_STEP3		3		/* エクセレント回帰曲線の計算ステップ 3 */
#define	REG_STEP4		4		/* エクセレント回帰曲線の計算ステップ 4 */
#define	LS_STEP1		1		/* 最小二乗法計算ステップ1 */
#define	LS_STEP2		2		/* 最小二乗法計算ステップ2 */
#define	LS_STEP3		3		/* 最小二乗法計算ステップ3 */
#define	EXCELLENT_CALC_INTERVAL	(F_SNS * 3)	/* 回帰曲線計算の間隔 */


#define	MAX_WEEK_NUM		1024			/* 1 cycle of GPS week number */


/***** エクセレント学習 ******/
#define	TIME_3MINUTE		(F_SNS*60*3)	/* 3分経過 */
#define	EXCELLENT_OFF		0		/* エクセレント補正なし */
#define	EXCELLENT_LEARN_ONLY	1		/* エクセレント学習あり、補正不可 */
#define	EXCELLENT_READY		2		/* エクセレント学習あり、補正可能 */
#define	EXCELLENT_TIME_LIMIT	7200		/* 前回電源OFF時間がこの時間[秒]以上なら エクセレント学習、補正を行う */
#define	GYRO_COOL_DOWN_TIME	3600		/* ジャイロが冷え切るまでの、前回電源OFF時間 */
#define	EXCELLENT_END1		(F_SNS*60*15)	/* 電源ON後この時間以上経過したらエクセレント学習を終了する */
#define	EXCELLENT_END2		(F_SNS*60*25)	/* 電源ON後この時間以内にエクセレント学習を終了する */
#define	MAX_SAMPLE_EXCELLENT	(EXCELLENT_END1/SAMPLE_INTERVAL)	/* 最大サンプル数 */
#define	MIN_SAMPLE_EXCELLENT	6		/* 最小サンプル数(次数+1以上であること)*/
#define	RECALC_DIFF_WEEK	6		/* 前回計算からこの週以上離れていたら再計算する */
#define	RECALC_SAMPLE_NUM	10		/* この回数以上サンプルがとれたら再計算する */
#define	MIN_PIVOT		0.001F		/* ガウス・ジョルダン法のピボット選択最小値 */


/* ジャイロオフセット保存用構造体 */
typedef struct  {
	FLOAT   Offset;				/* オフセット */
	DWORD   Time;				/* オフセット保存時刻(SnsTimer) */
} sGyroOffset;

/* オフセットサンプリング用構造体 */
typedef struct  {
	BYTE    OffsetPtr;			/* 現在のオフセット保存位置 */
	BYTE    OffsetJudgeCnt;		/* 保存したgyro平均化処理のためのカウンタ*/
	BYTE    LearnCnt;			/* オフセット学習回数 0～MAX_SAMPLE_DYNAMIC */
	BYTE    GetNewSample;		/* 新しいオフセットを取得したことを示すフラグ */
	BYTE    MaxSampleNum;		/* サンプリングする最大値 */
} sOffsetSample;

/* エクセレント学習構造体 */
typedef struct  {
	sGyroOffset		SampleOfs[MAX_SAMPLE_EXCELLENT];	/* 停止時のオフセットを保存 */
	sOffsetSample	SampleData;	/* オフセットサンプリング用構造体 */
	FLOAT   ExcellentOffset;		/* 走行時に使用したエクセレント補正済みオフセット値 */
	FLOAT   LastEstimate;		/* 前回のエクセレント予測オフセット */
	SHORT   GpsWeekNumber;		/* 現在のGPS週番号 -1:エラー */
	SHORT   OffsetErr;			/* オフセットの誤差の符号の積算値 */
	BYTE    LearnFinish;		/* オフセットサンプリングが終了したかを示すフラグ */
	BYTE    RegressStatus;		/* 回帰曲線計算ステータス */
} sExcellentLearn;

/*** エクセレント学習構造体 ****/
sExcellentLearn	ExcellentLearn;



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

/* センサバックアップ変数構造体 拡張部分の宣言 */
typedef struct  {
	//sSpeedBackupExtension	Speed;
	sGyroBackupExtension	Gyro;
	//sGSnsBackupExtension	GSns;
} sSensorBackupExtension;

#define SNS_GLOBAL
SNS_GLOBAL	sSensorBackupExtension	*SnsBkE;	/* センサバックアップ変数構造体 拡張部分へのポインタ */

VOID ExcellentLineFit(VOID)
{
	INT	i = 0;		/* ループカウンタ */
	INT	j = 0;		/* ループカウンタ */
	INT	k = 0;		/* ループカウンタ */
	INT	diff_week = 0;		/* 週番号の差 */
	INT	max_col = 0;		/* ピボット選択時のピボット最大行 */
	FLOAT	max_piv = 0.0F;	/* 最大ピボット値 */
	FLOAT	pivot = 0.0F;		/* ピボット値 */
	FLOAT	tempf = 0.0F;		/* テンポラリデータ */
	static	FLOAT	SMatrix[2*EXCELLENT_DIM+1];	/* 係数行列計算用テンポラリ配列 */
	static	FLOAT	TMatrix[EXCELLENT_DIM+1];	/* 係数行列計算用テンポラリ配列 */
	static	FLOAT	CMatrix[EXCELLENT_DIM+1][EXCELLENT_DIM+2];	/* 回帰曲線計算のための係数行列 */
	static	BYTE	GaussJordanCnt = 0;			/* ガウス・ジョルダン法によるはき出し実行回数 */
	static	BYTE	LeastSquaresStatus = 0;		/* 最小二乗法による係数行列作成カウンタ */
	static	BYTE	LsProcessedCnt = 0;			/* 処理済カウンタ */ 

	switch (ExcellentLearn.RegressStatus)  {
		/* 回帰曲線を計算するに値するかを判断 */
case REG_STEP1:
	/*** 回帰曲線を計算しない条件 ***/
	/* 最小サンプル数より小さいかチェック */
	if ((ExcellentLearn.SampleData.LearnCnt < MIN_SAMPLE_EXCELLENT)
		/* 現在の週番号が不明な場合 */
		|| (ExcellentLearn.GpsWeekNumber < 0))  {
			/* 回帰曲線を計算しない */
			ExcellentLearn.RegressStatus = REG_NOT_CALC;
			break;
	}
	/*** 回帰曲線を計算する条件 ***/
	/* エクセレント学習データが存在しない場合は、計算を開始する */
	if (SnsBkE->Gyro.ExcellentQuality == 0)  {
		/* 回帰曲線を計算する */
		ExcellentLearn.RegressStatus = REG_STEP2;
		/* 最小二乗法による係数行列作成カウンタクリア */
		LeastSquaresStatus = LS_STEP1;
		break;
	}
	/* 現在のエクセレント補正関数を作成してからの経過週を計算する */
	diff_week = ExcellentLearn.GpsWeekNumber - SnsBkE->Gyro.ExcellentWeek;
	/* GPS Week Number Roll Overの処理 */
	if (diff_week < 0)  {
		diff_week += MAX_WEEK_NUM;
	}
	/* 6週間以上計算していなかったら計算を更新する */
	if ((diff_week > RECALC_DIFF_WEEK)
		/* 10サンプル以上取得できたら、再計算する */
		|| (ExcellentLearn.SampleData.LearnCnt >= RECALC_SAMPLE_NUM)
		/* 現在のエクセレント補正関数のサンプル数より多い */
		|| (ExcellentLearn.SampleData.LearnCnt >= SnsBkE->Gyro.ExcellentQuality))  {
			/* 回帰曲線を計算する */
			ExcellentLearn.RegressStatus = REG_STEP2;
			/* 最小二乗法による係数行列作成カウンタクリア */
			LeastSquaresStatus = LS_STEP1;
			break;
	}
	/* 回帰曲線を計算しない */
	ExcellentLearn.RegressStatus = REG_NOT_CALC;
	break;

	/* 回帰曲線計算開始, 最小2乗法の係数行列を計算する */
case REG_STEP2:
	switch (LeastSquaresStatus)  {
case LS_STEP1:
	/* 最小二乗法による係数行列作成カウンタが0の場合 */
	/* 係数行列を初期化 */
	for (i=0; i<=2*EXCELLENT_DIM; i++)  {
		SMatrix[i] = 0.0F;
	}
	for (i=0; i<=EXCELLENT_DIM; i++)  {
		TMatrix[i] = 0.0F;
	}

	/* 次の計算へ */
	LeastSquaresStatus = LS_STEP2;
	/* 処理済カウンタをクリア */
	LsProcessedCnt = 0;
	break;

case LS_STEP2:
	/*** 係数行列を作成その1 ***/
	/* サンプル時刻のn乗を代入する変数をクリア */
	tempf = 1.0F;
	/* s0からs2mの計算 */
	for (i=0; i<=2*EXCELLENT_DIM; i++)  {
		SMatrix[i] += tempf;
		/* サンプル時刻のn乗を計算 */
		tempf *= (FLOAT)ExcellentLearn.SampleOfs[LsProcessedCnt].Time;
	}

	/*** 係数行列を作成その2 ***/
	/* サンプル時刻のn乗を代入する変数をクリア */
	tempf = 1.0F;
	/* t0からtmの計算 */
	for (i=0; i<=EXCELLENT_DIM; i++)  {
		TMatrix[i] += tempf * ExcellentLearn.SampleOfs[LsProcessedCnt].Offset;
		/* サンプル時刻のn乗を計算 */
		tempf *= (FLOAT)ExcellentLearn.SampleOfs[LsProcessedCnt].Time;
	}

	/* 次の計算に行く判断 */
	if (++LsProcessedCnt >= ExcellentLearn.SampleData.LearnCnt)  {
		/* 次の計算へ */
		LeastSquaresStatus = LS_STEP3;
	}
	break;

case LS_STEP3:
	/* 係数行列を代入 */
	for (i=0; i<=EXCELLENT_DIM; i++)  {
		for (j=0; j<=EXCELLENT_DIM; j++)  {
			CMatrix[i][j] = SMatrix[i+j];
		}
		CMatrix[i][EXCELLENT_DIM+1] = TMatrix[i];
	}
	/* 回帰曲線の係数行列計算終了 */
	ExcellentLearn.RegressStatus = REG_STEP3;
	/* ガウス・ジョルダン法によるはき出し実行回数初期化 */
	GaussJordanCnt = 0;
	break;

default :
	/* エクセレント回帰曲線計算終了 */
	ExcellentLearn.RegressStatus = REG_NOT_CALC;
	break;
	}
	break;

	/* ガウス・ジョルダン法により掃き出しを行い、連立方程式を解く */
case REG_STEP3:
	k = GaussJordanCnt;
	/*** ピボット選択 ***/
	max_piv = 0.0F;
	max_col = 0;
	/* 最大のピボットを選択する */
	for (i=k; i<=EXCELLENT_DIM; i++)  {
		if (fabs(CMatrix[i][k]) > max_piv)  {
			max_piv = static_cast< FLOAT >(fabs(CMatrix[i][k]));
			max_col = i;
		}
	}
	/* ピボットの最大値がスレ以上かチェック */
	if (max_piv < MIN_PIVOT)  {
		/* ピボットが小さすぎるため、計算不可能 */
		/* エクセレント回帰曲線計算終了 */
		ExcellentLearn.RegressStatus = REG_NOT_CALC;
		break;
	}
	/* ピボット最大の行と入れ替える */
	for (i=0; i<=EXCELLENT_DIM+1; i++)  {
		tempf = CMatrix[k][i];
		CMatrix[k][i] = CMatrix[max_col][i];
		CMatrix[max_col][i] = tempf;
	}
	/*** ガウス・ジョルダン法により掃き出しを行う ***/
	/* ピボット係数 */
	pivot = CMatrix[k][k];
	/* ピボット行をピボット係数で割る */
	for (i=k; i<=EXCELLENT_DIM+1; i++)  {
		CMatrix[k][i] = CMatrix[k][i] / pivot;
	}
	/* ピボット列の掃き出し*/
	for (i=0; i<=EXCELLENT_DIM; i++)  {
		if (i != k)  {
			tempf = CMatrix[i][k];
			for (j=k; j<=EXCELLENT_DIM+1; j++)  {
				CMatrix[i][j] = CMatrix[i][j] - tempf * CMatrix[k][j];
			}
		}
	}
	/* ガウス・ジョルダン法によるはき出し実行回数インクリメント */
	GaussJordanCnt++;
	/* 0～計算次数まで終了したか？(次数+1回) */
	if (GaussJordanCnt > EXCELLENT_DIM)  {
		/* ガウス・ジョルダン法によるはき出し終了 */
		ExcellentLearn.RegressStatus = REG_STEP4;
	}
	/* 関数終了 */
	break;

	/* 計算結果を一度にバックアップデータに保存する */
case REG_STEP4:
	/* エクセレント補正関数のクオリティ 0:関数なし */
	SnsBkE->Gyro.ExcellentQuality = ExcellentLearn.SampleData.LearnCnt;
	/* エクセレントを学習した週 */
	SnsBkE->Gyro.ExcellentWeek = ExcellentLearn.GpsWeekNumber;
	/* エクセレント補正の終了時間(最後のサンプルポイントの時刻を代入) */
	SnsBkE->Gyro.ExcellentLimit = ExcellentLearn.SampleOfs[ExcellentLearn.SampleData.OffsetPtr].Time;
	/* エクセレント補正の回帰曲線の係数 */
	for (i=0; i<=EXCELLENT_DIM; i++)  {
		/* 計算したオフセットの回帰曲線の係数を代入 */
		SnsBkE->Gyro.ExcellentFunction[i] = CMatrix[i][EXCELLENT_DIM+1];
	}
	/* エクセレント学習をしたときのオフセットの平均 */
	tempf = 0.0F;
	for (i=0; i<ExcellentLearn.SampleData.LearnCnt; i++)  {
		tempf += ExcellentLearn.SampleOfs[i].Offset;
	}
	SnsBkE->Gyro.ExcellentAve = tempf / ExcellentLearn.SampleData.LearnCnt;
	/* エクセレント回帰曲線計算終了 */
	ExcellentLearn.RegressStatus = REG_NOT_CALC;
	break;

default : 
	/* エクセレント回帰曲線計算終了 */
	ExcellentLearn.RegressStatus = REG_NOT_CALC;
	break;
	}
}



int main()
{

	return 0;
}