#include <iostream>
#include <Windows.h>
using namespace std;


//以下 LOCSAVE.DATの構造体
#pragma pack(push, 1)
// 自車位置保存データファイルヘッダ
struct LocEntSaveFileHeader
{
	BYTE						byVersion;					// バージョン
	BYTE						byReserve[3];				// リザーブ
};

// 自車位置保存データセクションヘッダ
struct LocEntSaveSectHeader
{
	WORD						wSectionId;					// セクションID
	WORD						wSize;						// データサイズ
	DWORD						dwCrc32;					// チェックサム（CRC32）
};

// GPS・センサ出力情報
struct LocEntSaveGpsSnsInfo
{
	BYTE						byVersion;					// バージョン
	BYTE						byReserve[3];				// リザーブ
	SHORT						nHeading;					// センサ出力方位。メールデータの形のまま代入します。
	SHORT						nIncAngle;					// センサ出力傾斜角度。メールデータの形のまま代入します。
	DOUBLE						dTripMeter;					// センサトリップメーター。メールデータの形のまま代入します。
};
#pragma pack(pop)
//以上 LOCSAVE.DATの構造体

//LOCSAVE.DAT(LOCから提供されるファイル)
const TCHAR NP_GPS_LOCSAVEDAT[]   =	TEXT( "D:\\LOCSAVE.DAT" );

typedef BOOL NP_BOOL;
#define NP_TRUE TRUE
#define NP_FALSE FALSE

/******************************************************************************
  【処理】LOCSAVE.DAT情報取得・設定処理
  【戻値】なし
  【解説】LOCから提供されるLOCSAVE.DATから情報を取得し設定する
******************************************************************************/
VOID 
GetLocSaveData(VOID)
{
	FILE					*file;					// ファイル
	LocEntSaveFileHeader	file_header;			// ファイルヘッダ
	LocEntSaveSectHeader	sect_header;			// データヘッダ
	LocEntSaveGpsSnsInfo	save_gpssns;			// GPSSNS情報
	DWORD					err_code = 0x00;
	NP_BOOL					bErrFlag = NP_FALSE;

	// 初期化
	memset(&file_header, 0, sizeof(file_header));
	memset(&sect_header, 0, sizeof(sect_header));
	memset(&save_gpssns, 0, sizeof(save_gpssns));

	// LOCSAVE.DATファイルオープン
	if ((file = fopen( NP_GPS_LOCSAVEDAT, "r" )) == NULL) {
		return;
	}

	// ファイルヘッダ読み込み
	// フラグ初期化
	bErrFlag = NP_FALSE;
	fread(&file_header, sizeof(file_header), 1, file);
	if ( 0 == file_header.byVersion ) {
		// ファイルバージョンOK
		bErrFlag = NP_TRUE;
	}

	if ( NP_TRUE == bErrFlag ) {
		// ファイルヘッダ読み込み成功
		// フラグ初期化
		bErrFlag = NP_FALSE;
		// GPSSNSのセクションまでループ
		do
		{
			// セクションヘッダ読み込み
			fread(&sect_header, sizeof(sect_header), 1, file);

			// GPSSNSのセクションの場合
			if ( 0x8000 == sect_header.wSectionId ) {
				// データサイズチェック
				if (sect_header.wSize == sizeof(save_gpssns)) {
					// データサイズOK
					bErrFlag = NP_TRUE;
				}
				break;
			}

			// 今回のセクションのデータ分飛ばす
			fseek(file, sect_header.wSize, SEEK_CUR) ;

		}while (true);
	}

	if ( NP_TRUE == bErrFlag ) {
		// GPSSNSセクションヘッダ読み込み成功
		// フラグ初期化
		bErrFlag = NP_FALSE;
		// GPSSNSデータ読み込み
		fread(&save_gpssns, sizeof(save_gpssns), 1, file);
	}

	if ( NP_TRUE == bErrFlag ) {
		//// LOCSAVE.DATファイル情報読み込み成功
		///* センサ出力方位データ */
		//FLOAT fDirection = 0.0F;
	 //   /* 方位をナビ方位から、北0度時計回りに変換する */
		//fDirection = 90.0F - (static_cast< FLOAT >(save_gpssns.nHeading) / DEG2WORD);
	 //   /* 方位修正: 0.0[deg]～360.0[deg] */
	 //   MOD(fDirection, 0.0F, 360.0F);
		///* センサ出力方位セット */
		//SensorBackupBasic.Gyro.Direction = fDirection;

		///* センサ出力傾斜角度セット */
		//SensorBackupExtension.GSns.SlopeAngle = static_cast< FLOAT >(save_gpssns.nIncAngle) / 100.0F;

		///* センサトリップメータセット */
		//SnsBkM.Odometer = save_gpssns.dTripMeter;
	}

	// ファイルクローズ
	fclose( file );
	return;

}


int main()
{
	GetLocSaveData();

	return 0;
}