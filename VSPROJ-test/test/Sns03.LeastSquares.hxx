#include <Windows.h>

#include <iostream>
#include <cmath>

using namespace std;

#define	PRD_LEN_EX_BUF_SIZE	7		/* 速度に対する距離学習値保存用構造体のバッファ数 */
#define	PRD_LEN_CORR_BUF_SIZE	4		/* 距離学習値近似曲線係数格納バッファ数 */

/* 距離学習値近似曲線用構造体の宣言 */
typedef struct  {
    FLOAT   PrdLengthCorr[PRD_LEN_CORR_BUF_SIZE];	/* 距離学習値近似曲線係数格納バッファ */
    BYTE    PrdLengthProcess;	/* 距離学習値近似曲線算出ステータス */
    BYTE    UseBufCnt;		/* 距離学習値近似曲線算出使用バッファ数 */
    BYTE    PrdLengthOk;	/* 距離学習値近似曲線使用可能フラグ */
} sApproxPeriodLengthData;

/* 速度に対する距離学習値保存用構造体の宣言 */
typedef struct  {
    FLOAT   PrdLength;		/* 速度に対する距離学習値 */
    FLOAT   SpeedAve;		/* 距離学習時の平均速度 */
    WORD    PrdLengthCnt;	/* 各速度の距離学習回数 */
} sPeriodLengthExtension;

#define	APX_PRD_LEN_DIM		3		/* 複数距離学習値による距離学習値近似曲線の次数 */
#define	APX_LEARN_OFF		0		/* 距離学習値近似曲線の計算を行わない */
#define	APX_LEARN_STEP1		1		/* 距離学習値近似曲線の計算ステップ 1 */
#define	APX_LEARN_STEP2		2		/* 距離学習値近似曲線の計算ステップ 2 */
#define	APX_LEARN_STEP3		3		/* 距離学習値近似曲線の計算ステップ 3 */
#define	APX_LEARN_STEP4		4		/* 距離学習値近似曲線の計算ステップ 4 */
#define	LEARN_LS_STEP1		1		/* 最小二乗法計算ステップ1 */
#define	LEARN_LS_STEP2		2		/* 最小二乗法計算ステップ2 */
#define	LEARN_LS_STEP3		3		/* 最小二乗法計算ステップ3 */

#define	APX_LEARN_CNT_OK	50		/* 近似曲線算出使用可能学習回数[回] */
#define	APX_USE_BUF_4		4		/* 近似曲線算出使用バッファ数4[個] */
#define	APX_USE_BUF_5		5		/* 近似曲線算出使用バッファ数5[個]*/
#define	APX_USE_BUF_6		6		/* 近似曲線算出使用バッファ数6[個]*/
#define	APX_USE_BUF_7		7		/* 近似曲線算出使用バッファ数7[個]*/
#define	APX_PRD_MIN_PIVOT       0.001F		/* ガウス・ジョルダン法のピボット選択最小値 */


#define	SNS_GLOBAL_B
/* 距離学習値近似曲線用構造体 */
SNS_GLOBAL_B	sApproxPeriodLengthData	ApxPeriodLengthData;
SNS_GLOBAL_B	sPeriodLengthExtension	PeriodLengthEx[PRD_LEN_EX_BUF_SIZE];

VOID PrdLenLeastSquares(VOID)
{
	INT	    i = 0;	    /* ループカウンタ */
	INT	    j = 0;	    /* ループカウンタ */
	INT	    k = 0;	    /* ループカウンタ */
	INT	    max_col = 0;	/* ピボット選択時のピボット最大行 */
	FLOAT   max_piv = 0.0F;	/* 最大ピボット値 */
	FLOAT   pivot = 0.0F;	/* ピボット値 */
	FLOAT   tempf = 0.0F;	/* テンポラリデータ */
	static  FLOAT   SMatrix[2*APX_PRD_LEN_DIM+1];	/* 係数行列計算用テンポラリ配列 */
	static  FLOAT   TMatrix[APX_PRD_LEN_DIM+1];	/* 係数行列計算用テンポラリ配列 */
	static  FLOAT   CMatrix[APX_PRD_LEN_DIM+1][APX_PRD_LEN_DIM+2];	/* 近似曲線計算のための係数行列 */
	static  BYTE    GaussJordanCnt = 0;			/* ガウス・ジョルダン法によるはき出し実行回数 */
	static  BYTE    LeastSquaresStatus = 0;		/* 最小二乗法による係数行列作成カウンタ */
	static  BYTE    LsProcessedCnt = 0;			/* 処理済カウンタ */

	/* 距離学習値近似曲線算出ステータスにより各処理を行う */
	switch (ApxPeriodLengthData.PrdLengthProcess)  {
		/* 近似曲線を計算するに値するかを判断 */
	case APX_LEARN_STEP1:
		/*** 近似曲線を計算する条件 ***/
		/* 距離学習値近似曲線算出使用バッファ数をクリア */
		ApxPeriodLengthData.UseBufCnt = 0;
		/* 速度ごとの距離学習値が使用可能かチェック */
		for (i=0; i<PRD_LEN_EX_BUF_SIZE; i++)  {
			if (PeriodLengthEx[i].PrdLengthCnt < APX_LEARN_CNT_OK)  {    /* 速度ごとの距離学習回数が閾値以下 */
				/* 距離学習値近似曲線使用バッファ数のカウント終了 */
				break;
			}
			/* 距離学習値近似曲線算出使用バッファ数をインクリメント */
			ApxPeriodLengthData.UseBufCnt++;
		}

		/* 近似曲線算出使用可能バッファ数チェック */
		if (ApxPeriodLengthData.UseBufCnt < APX_USE_BUF_4)  {
			/* 距離学習近似曲線算出ステータスを算出を行わないよう変更 */
			ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_OFF;
			/* 距離学習値近似曲線算出使用バッファ数をクリア */
			ApxPeriodLengthData.UseBufCnt = 0;
			/* 距離学習近似曲線の計算を行わない */
			break;
		}

		/* 近似曲線を計算する */
		ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_STEP2;
		/* 最小二乗法による係数行列作成カウンタクリア */
		LeastSquaresStatus = LEARN_LS_STEP1;
		break;
	case APX_LEARN_STEP2:
		switch (LeastSquaresStatus)  {
	case LEARN_LS_STEP1:
		/* 最小二乗法による係数行列作成カウンタが0の場合 */
		/* 係数行列を初期化 */
		for (i=0; i<=2*APX_PRD_LEN_DIM; i++)  {
			SMatrix[i] = 0.0F;
		}
		for (i=0; i<=APX_PRD_LEN_DIM; i++)  {
			TMatrix[i] = 0.0F;
		}

		/* 次の計算へ */
		LeastSquaresStatus = LEARN_LS_STEP2;
		/* 処理済カウンタをクリア */
		LsProcessedCnt = 0;
		break;

	case LEARN_LS_STEP2:
		/*** 係数行列を作成その1 ***/
		/* 速度のn乗を代入する変数をクリア */
		tempf = 1.0F;
		/* s0からs2mの計算 */
		for (i=0; i<=2*APX_PRD_LEN_DIM; i++)  {
			SMatrix[i] += tempf;
			/* 速度のn乗を計算 */
			tempf *= PeriodLengthEx[LsProcessedCnt].SpeedAve;
		}

		/*** 係数行列を作成その2 ***/
		/* 速度のn乗を代入する変数をクリア */
		tempf = 1.0F;
		/* t0からtmの計算 */
		for (i=0; i<=APX_PRD_LEN_DIM; i++)  {
			TMatrix[i] += tempf * PeriodLengthEx[LsProcessedCnt].PrdLength;
			/* 速度のn乗を計算 */
			tempf *=  PeriodLengthEx[LsProcessedCnt].SpeedAve;
		}

		/* 次の計算に行く判断 */  //pset 累赘，前面至少4次，这里又要至少4次
		if (++LsProcessedCnt >= ApxPeriodLengthData.UseBufCnt)  {
			/* 次の計算へ */
			LeastSquaresStatus = LEARN_LS_STEP3;
		}
		break;

	case LEARN_LS_STEP3:
		/* 係数行列を代入 */
		for (i=0; i<=APX_PRD_LEN_DIM; i++)  {
			for (j=0; j<=APX_PRD_LEN_DIM; j++)  {
				CMatrix[i][j] = SMatrix[i+j];
			}
			CMatrix[i][APX_PRD_LEN_DIM+1] = TMatrix[i];
		}
		/* 近似曲線の係数行列計算終了 */
		ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_STEP3;
		/* ガウス・ジョルダン法によるはき出し実行回数初期化 */
		GaussJordanCnt = 0;
		break;

	default :
		/* 距離学習値近似曲線計算終了 */
		ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_OFF;
		break;
		}
		break;
		/* ガウス・ジョルダン法により掃き出しを行い、連立方程式を解く */
	case APX_LEARN_STEP3:
		k = GaussJordanCnt;
		/*** ピボット選択 ***/
		max_piv = 0.0F;
		max_col = 0;
		/* 最大のピボットを選択する */
		for (i=k; i<=APX_PRD_LEN_DIM; i++)  {
			if (fabs(CMatrix[i][k]) > max_piv)  {
				max_piv = static_cast< FLOAT >(fabs(CMatrix[i][k]));
				max_col = i;
			}
		}
		/* ピボットの最大値がスレ以上かチェック */
		if (max_piv < APX_PRD_MIN_PIVOT)  {
			/* ピボットが小さすぎるため、計算不可能 */
			/* 距離学習値近似曲線計算終了 */
			ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_OFF;
			break;
		}
		/* ピボット最大の行と入れ替える */
		for (i=0; i<=APX_PRD_LEN_DIM+1; i++)  {
			tempf = CMatrix[k][i];
			CMatrix[k][i] = CMatrix[max_col][i];
			CMatrix[max_col][i] = tempf;
		}
		/*** ガウス・ジョルダン法により掃き出しを行う ***/
		/* ピボット係数 */
		pivot = CMatrix[k][k];
		/* ピボット行をピボット係数で割る */
		for (i=k; i<=APX_PRD_LEN_DIM+1; i++)  {
			CMatrix[k][i] = CMatrix[k][i] / pivot;
		}
		/* ピボット列の掃き出し*/
		for (i=0; i<=APX_PRD_LEN_DIM; i++)  {
			if (i != k)  {
				tempf = CMatrix[i][k];
				for (j=k; j<=APX_PRD_LEN_DIM+1; j++)  {
					CMatrix[i][j] = CMatrix[i][j] - tempf * CMatrix[k][j];
				}
			}
		}
		/* ガウス・ジョルダン法によるはき出し実行回数インクリメント */
		GaussJordanCnt++;
		/* 0～計算次数まで終了したか？(次数+1回) */
		if (GaussJordanCnt > APX_PRD_LEN_DIM)  {
			/* ガウス・ジョルダン法によるはき出し終了 */
			ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_STEP4;
		}
		/* 関数終了 */
		break;

		/* 計算結果を一度にバックアップデータに保存する */
	case APX_LEARN_STEP4:
		/* 距離学習値補正の近似曲線の係数 */
		for (i=0; i<=APX_PRD_LEN_DIM; i++)  {
			/* 計算したオフセットの近似曲線の係数を代入 */
			ApxPeriodLengthData.PrdLengthCorr[i] = CMatrix[i][APX_PRD_LEN_DIM+1];
		}

		/* 距離学習値近似曲線使用可能フラグを立てる */
		ApxPeriodLengthData.PrdLengthOk = TRUE;

		/* 距離学習値近似曲線計算終了 */
		ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_OFF;
		break;

	default : 
		/* 距離学習値近似曲線計算終了 */
		ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_OFF;
		break;
	}
}


FLOAT ApxPeriodLengthCalc(
	FLOAT	apx_calc_speed	/* 近似式代入用速度 */
	)
{
	FLOAT   temp_prd_len = 0.0F;    /* 近似距離学習値格納用変数 */
	FLOAT   tempf = 1.0F;	    /* 近似距離学習値算出用変数 */
	INT	    i = 0;		    /* カウンタ */

	/* 近似距離学習値を初期化 */
	temp_prd_len = ApxPeriodLengthData.PrdLengthCorr[0];

	/* 近似距離学習値を計算 */
	for (i=1; i<=APX_PRD_LEN_DIM; i++)  {
		/* 近似用速度のn乗を計算 */
		tempf *= apx_calc_speed;
		/* 近似距離学習値の計算 */
		temp_prd_len += ApxPeriodLengthData.PrdLengthCorr[i] * tempf;
	}

	/* 近似距離学習値を分周比倍する */
	//temp_prd_len = temp_prd_len * SnsBkB->Speed.PeriodDivision;
	temp_prd_len = temp_prd_len * 1; //分周比设为1

	return (temp_prd_len);
}


int main()
{
	ApxPeriodLengthData.PrdLengthProcess = APX_LEARN_STEP1;
	int i = 0;
	const int NUM = 4;
	for (i=0; i<NUM; i++)  {
		PeriodLengthEx[i].PrdLengthCnt = 100; // >=50
	}

	PeriodLengthEx[0].SpeedAve = 55.1F;
	PeriodLengthEx[0].PrdLength = 70.0F;
	

	PrdLenLeastSquares(); //step 1;

	PrdLenLeastSquares(); //step 2.1;
	for (i=0; i<NUM; i++)  {
		PrdLenLeastSquares(); //step 2.2; //pset 很蛋疼，重复UseBufCnt次，只为改变SMatrix[0]的值为UseBufCnt
	}
	PrdLenLeastSquares(); //step 2.3;

	for (i=0; i<APX_PRD_LEN_DIM+1; i++)  {
		PrdLenLeastSquares(); //step 3; //pset 高斯乔丹法，迭代消元，迭代了4次
	}

	PrdLenLeastSquares(); //step 4;


	if (ApxPeriodLengthData.PrdLengthOk == TRUE)
	{
		//cout factor
		for (i=0; i<=APX_PRD_LEN_DIM; i++)  {
			cout << ApxPeriodLengthData.PrdLengthCorr[i] << "  " ;
		}
		cout << endl;

		//calc speed=35;
		FLOAT ApxPeriodLength = ApxPeriodLengthCalc(70); 
		//FLOAT PulseLen = ApxPeriodLength / static_cast< FLOAT >(SnsBkB->Speed.PeriodDivision);
		FLOAT PulseLen = ApxPeriodLength / 1.0f;
		cout << PulseLen  << endl;
	}

	return 0;
}