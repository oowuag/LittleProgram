#include <iostream>
#include <Windows.h>
using namespace std;

/* Local測地系パラメータ構造体 */
typedef	struct	{
    FLOAT   delta_a;
    FLOAT   delta_f;
    FLOAT   delta_x;
    FLOAT   delta_y;
    FLOAT   delta_z;
} sDatumParameter;

/* 測地系変換パラメータ(Local測地系パラメータ、基準楕円体パラメータ)構造体 */
typedef	struct	{
    DOUBLE   delta_a;
    DOUBLE   delta_f;
    DOUBLE   delta_x;
    DOUBLE   delta_y;
    DOUBLE   delta_z;
    DOUBLE   local_flat;
    DOUBLE   local_axis;
    DOUBLE   one_me2;
    DOUBLE   ecc2;
} sDatumEllipseParms;

const	sDatumParameter	LocalPrmTable[] = {
/*<	delta_a 	delta_f 	dx	dy	dz	>*/

/*  0: "WGS84", "WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  1: "ADINDAN"  "CLARKE1880"	*/
	112.145F,	0.54750714F,	162.0F,	12.0F,	-206.0F,
/*  2: "ARC 1950" "CLARKE1880"	*/
	112.145F,	0.54750714F,	143.0F,	90.0F,	294.0F,
/*  3: "AUSTRALIAN GEODETIC 1966" "AUSTRALIAN NATIONAL" */
	23.0F,		0.00081204F,	133.0F,	48.0F,	-148.0F,
/*  4: "BUKIT RIMPAH" "BESSEL"	*/
	-739.845F,	-0.10037483F,	384.0F,	-664.0F, 48.0F,
/*  5: "CAMP AREA ASTRO" "INTERNATIONAL" */
	251.0F,		0.14192702F,	104.0F,	129.0F,	-239.0F,
/*  6: "DJAKARTA","BESSEL" */
	-739.845F,	-0.10037483F,	377.0F,	-681.0F, 50.0F,
/*  7: "EUROPEAN 1950","INTERNATIONAL */
	251.0F,		0.14192702F,	87.0F,	98.0F,	121.0F,
/*  8: "GEODETIC DATUM 1949","INTERNATIONAL */
	251.0F,		0.14192702F,	-84.0F,	22.0F,	-209.0F,
/*  9: "GHANA","WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  10: "GUAM 1963","CLARKE 1866 */
	69.4F,		0.37264639F,	100.0F,	248.0F,	-259.0F,
/*  11: "G.SEGARA","BESSEL */
	-739.845F,	-0.10037483F,	403.0F,	-684.0F, -41.0F,
/*  12: "G.SERINDUNG","WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  13: "HERAT NORTH","INTERNATIONAL */
	251.0F,		0.14192702F,	333.0F,	222.0F,	-114.0F,
/*  14: "HJORSEY 1955","INTERNATIONAL */
	251.0F,		0.14192702F,	73.0F,	-46.0F,	86.0F,
/*  15: "HU-TZU-SHAN","INTERNATIONAL */
	251.0F,		0.14192702F,	634.0F,	549.0F,	201.0F,
/*  16: "INDIAN","EVEREST */
	-860.655F,	-0.28361368F,	-173.0F, -750.0F, -264.0F,
/*  17: "IRELAND 1965","MODIFIED AIRY */
	-796.811F,	0.11960023F,	-506.0F, 122.0F,	-611.0F,
/*  18: "KERTAU","MODIFIED EVEREST */
	-832.937F,	-0.28361368F,	11.0F,	-851.0F, -5.0F,
/*  19: "LIBERIA 1964","CLARKE 1880 */
	112.145F,	0.54750714F,	90.0F,	-40.0F,	-88.0F,
/*  20: "USER ENTERED","USER" */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  21: "LUZON","CLARKE 1866 */
	69.4F,		0.37264639F,	133.0F,	77.0F,	54.0F,
/*  22: "MERCHICH","CLARKE 1880 */
	112.145F,	0.54750714F,	-31.0F,	-146.0F, -47.0F,
/*  23: "MONJONG LOWE", "WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  24: "NIGERIA", "CLARKE 1880 */
	112.145F,	0.54750714F,	92.0F,	93.0F,	-122.0F,
/*  25: "NORTH AMERICAN 1927(CONUS)", "CLARKE 1886 */
	69.4F,		0.37264639F,	8.0F,	-160.0F, -176.0F,
/*  26: "NORTH AMERICAN 1927(CANADA)", "CLARKE 1866 */
	69.4F,		0.37264639F,	9.0F,	-151.0F, -185.0F,
/*  27: "OLD HAWAIIAN,MAUI", "INTERNATIONAL */
	251.0F,		0.14192702F,	-210.0F, 230.0F,	357.0F,
/*  28: "OLD HAWAIIAN,OAHU", "INTERNATIONAL */
	251.0F,		0.14192702F,	-201.0F, 224.0F,	349.0F,
/*  29: "OLD HAWAIIAN,KAUAI", "INTERNATIONAL */
	251.0F,		0.14192702F,	-190.0F, 230.0F,	341.0F,
/*  30: "ORDNANCE SURVEY", "AIRY */
	-573.604F,	-0.11960023F,	-375.0F, 111.0F,	-431.0F,
/*  31: "QORNOQ", "INTERNATIONAL */
	251.0F,		0.14192702F,	-164.0F, -138.0F, 189.0F,
/*  32: "SIERRA LEONE 1960", "WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  33: "SOUTH AMERICAN(1956)", "INTERNATIONAL */
	251.0F,		0.14192702F,	288.0F,	-175.0F, 375.0F,
/*  34: "SOUTH AMERICAN(CORREGO)", "INTERNATIONAL */
	251.0F,		0.14192702F,	206.0F,	-172.0F, 6.0F,
/*  35: "SOUTH AMERICAN(CAMPO)", "INTERNATIONAL */
	251.0F,		0.14192702F,	148.0F,	-136.0F, 90.0F,
/*  36: "SOUTH AMERICAN(CHUA ASTRO)", "INTERNATIONAL */
	251.0F,		0.14192702F,	134.0F,	-229.0F, 29.0F,
/*  37: "SOUTH AMERICAN(YACARE)", "INTERNATIONAL */
	251.0F,		0.14192702F,	155.0F,	-171.0F, 37.0F,
/*  38: "TANANARIVE", "INTERNATIONAL */
	251.0F,		0.14192702F,	189.0F,	242.0F,	91.0F,
/*  39: "TIMBALAI", "BESSEL */
	-739.845F,	-0.10037483F,	639.0F,	-583.0F, 55.0F,
/*  40: "TOKYO", "BESSEL */
	-739.845F,	-0.10037483F,	146.43F, -507.89F, -681.46F,
/*  41: "VOIROL", "WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  42: "SD,MGR-RELATED,INDIAN", "EVEREST */
	-860.655F,	-0.28361368F,	-173.0F, -750.0F, -264.0F,
/*  43: "SD,LUZON", "CLARKE 1866 */
	69.4F,		0.37264639F,	133.0F,	77.0F,	54.0F,
/*  44: "SD,TOKYO", "BESSEL */
	-739.845F,	-0.10037483F,	128.0F,	-481.0F, -664.0F,
/*  45: "SD,WGS84", "WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
/*  46: "WGS72", "WGS 72 */
	-2.0F,		-0.0003121057F,	0.0F,	0.0F,	-4.5F,
/*  47: "WGS84", "WGS 84 */
	0.0F,		0.0F,		0.0F,	0.0F,	0.0F,
};

#define	GPS_GLOBAL_B

// 測地変換用定義
#define		PI		3.141592653589793
#define		D2R		(PI/180.0)
#define		R2D		(180.0/PI)

#define		A_TKY		6377397.155		// ベッセル楕円体長半径[m]
#define		F_TKY		(1.0/299.152813)	// ベッセル楕円体扁平度
#define		A_WGS84		6378137.0
#define		E2_WGS84	6.69437999013e-3
#define		F_WGS84		(1.0/298.257223563)
#define		ES_WGS84	(2.0 * F_WGS84 - F_WGS84 * F_WGS84)
#define		ADB_WGS84       (1.0 / (1.0 - F_WGS84)) // "a divided by b"

#define		DX_TKY		146.43
#define		DY_TKY		-507.89
#define		DZ_TKY		-681.46


/*** gps_local.cpp ***/

#define	USER_DEFINED		20			/* user defined datum parameter */
#ifdef	F_WGS84
#   undef F_WGS84
#endif
#define	F_WGS84			298.257223563

class NPSnsHandlerBase
{
private:
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
    /* ユーザ測地系パラメータ */
    GPS_GLOBAL_B	DOUBLE	bkUsrDeltaX;
    GPS_GLOBAL_B	DOUBLE	bkUsrDeltaY;
    GPS_GLOBAL_B	DOUBLE	bkUsrDeltaZ;
    GPS_GLOBAL_B	DOUBLE	bkUsrDeltaA;
    GPS_GLOBAL_B	DOUBLE	bkUsrDeltaF;
public:
	VOID LocalParms(
		SHORT	param		/* 測地系パラメータ */
		);
	VOID GetDatumEllipseParms(
		SHORT		param,	/* 測地系パラメータ */
		sDatumEllipseParms	*dp	/* 測地系変換パラメータ構造体のアドレス */
		);
};

/******************************************************************************
  【処理】測地系変換パラメータの計算
  【戻値】なし
  【解説】測地系変換パラメータの計算
******************************************************************************/
VOID NPSnsHandlerBase::LocalParms(
    SHORT	param		/* 測地系パラメータ */
    )
{
    sDatumEllipseParms	datum_parms;

    /* 初期化処理 */
    memset (&datum_parms, 0, sizeof( sDatumEllipseParms ) );

    /* 測地系変換パラメータを取得 */
    GetDatumEllipseParms(param, &datum_parms);

    /* 測地系変換パラメータをバックアップ変数にセット */
    bkDeltaA = datum_parms.delta_a;
    bkDeltaF = datum_parms.delta_f;
    bkDeltaX = datum_parms.delta_x;
    bkDeltaY = datum_parms.delta_y;
    bkDeltaZ = datum_parms.delta_z;
    bkLocalFlat = datum_parms.local_flat;
    bkLocalAxis = datum_parms.local_axis;
    bkEcc2 = datum_parms.ecc2;
    bkOneMe2 = datum_parms.one_me2;

    /* set geodetic parameter */
    bkGeodPrm = param;
}

/******************************************************************************
  【処理】測地系変換パラメータの計算
  【戻値】なし
  【解説】測地系変換パラメータの計算の計算を行い、渡された構造体にセットする
******************************************************************************/
VOID NPSnsHandlerBase::GetDatumEllipseParms(
    SHORT		param,	/* 測地系パラメータ */
    sDatumEllipseParms	*dp	/* 測地系変換パラメータ構造体のアドレス */
    )
{
    /* users local prm */
    if (param == USER_DEFINED)  {
	dp->delta_a = bkUsrDeltaA;
	dp->delta_f = bkUsrDeltaF;
	dp->delta_x = bkUsrDeltaX;
	dp->delta_y = bkUsrDeltaY;
	dp->delta_z = bkUsrDeltaZ;
    }
    /* use LocalPrmTable */
    else  {
	dp->delta_a = LocalPrmTable[param].delta_a;
	dp->delta_f = LocalPrmTable[param].delta_f;
	dp->delta_x = LocalPrmTable[param].delta_x;
	dp->delta_y = LocalPrmTable[param].delta_y;
	dp->delta_z = LocalPrmTable[param].delta_z;
    }

    /* 基準楕円体パラメータを計算 */
    dp->local_flat = 1.0 / F_WGS84 + dp->delta_f * 0.0001;
    dp->local_axis = A_WGS84 + dp->delta_a;
    dp->ecc2 = dp->local_flat * (2.0 - dp->local_flat);
    dp->one_me2 = 1.0 - dp->ecc2;
}


int main()
{
	NPSnsHandlerBase npBase;
	npBase.LocalParms(40); //TOKYO
	npBase.LocalParms(47); //WGS84

	return 0;
}
