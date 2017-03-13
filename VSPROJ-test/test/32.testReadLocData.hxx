#include <iostream>
#include <Windows.h>
using namespace std;


//�ȉ� LOCSAVE.DAT�̍\����
#pragma pack(push, 1)
// ���Ԉʒu�ۑ��f�[�^�t�@�C���w�b�_
struct LocEntSaveFileHeader
{
	BYTE						byVersion;					// �o�[�W����
	BYTE						byReserve[3];				// ���U�[�u
};

// ���Ԉʒu�ۑ��f�[�^�Z�N�V�����w�b�_
struct LocEntSaveSectHeader
{
	WORD						wSectionId;					// �Z�N�V����ID
	WORD						wSize;						// �f�[�^�T�C�Y
	DWORD						dwCrc32;					// �`�F�b�N�T���iCRC32�j
};

// GPS�E�Z���T�o�͏��
struct LocEntSaveGpsSnsInfo
{
	BYTE						byVersion;					// �o�[�W����
	BYTE						byReserve[3];				// ���U�[�u
	SHORT						nHeading;					// �Z���T�o�͕��ʁB���[���f�[�^�̌`�̂܂ܑ�����܂��B
	SHORT						nIncAngle;					// �Z���T�o�͌X�Ίp�x�B���[���f�[�^�̌`�̂܂ܑ�����܂��B
	DOUBLE						dTripMeter;					// �Z���T�g���b�v���[�^�[�B���[���f�[�^�̌`�̂܂ܑ�����܂��B
};
#pragma pack(pop)
//�ȏ� LOCSAVE.DAT�̍\����

//LOCSAVE.DAT(LOC����񋟂����t�@�C��)
const TCHAR NP_GPS_LOCSAVEDAT[]   =	TEXT( "D:\\LOCSAVE.DAT" );

typedef BOOL NP_BOOL;
#define NP_TRUE TRUE
#define NP_FALSE FALSE

/******************************************************************************
  �y�����zLOCSAVE.DAT���擾�E�ݒ菈��
  �y�ߒl�z�Ȃ�
  �y����zLOC����񋟂����LOCSAVE.DAT��������擾���ݒ肷��
******************************************************************************/
VOID 
GetLocSaveData(VOID)
{
	FILE					*file;					// �t�@�C��
	LocEntSaveFileHeader	file_header;			// �t�@�C���w�b�_
	LocEntSaveSectHeader	sect_header;			// �f�[�^�w�b�_
	LocEntSaveGpsSnsInfo	save_gpssns;			// GPSSNS���
	DWORD					err_code = 0x00;
	NP_BOOL					bErrFlag = NP_FALSE;

	// ������
	memset(&file_header, 0, sizeof(file_header));
	memset(&sect_header, 0, sizeof(sect_header));
	memset(&save_gpssns, 0, sizeof(save_gpssns));

	// LOCSAVE.DAT�t�@�C���I�[�v��
	if ((file = fopen( NP_GPS_LOCSAVEDAT, "r" )) == NULL) {
		return;
	}

	// �t�@�C���w�b�_�ǂݍ���
	// �t���O������
	bErrFlag = NP_FALSE;
	fread(&file_header, sizeof(file_header), 1, file);
	if ( 0 == file_header.byVersion ) {
		// �t�@�C���o�[�W����OK
		bErrFlag = NP_TRUE;
	}

	if ( NP_TRUE == bErrFlag ) {
		// �t�@�C���w�b�_�ǂݍ��ݐ���
		// �t���O������
		bErrFlag = NP_FALSE;
		// GPSSNS�̃Z�N�V�����܂Ń��[�v
		do
		{
			// �Z�N�V�����w�b�_�ǂݍ���
			fread(&sect_header, sizeof(sect_header), 1, file);

			// GPSSNS�̃Z�N�V�����̏ꍇ
			if ( 0x8000 == sect_header.wSectionId ) {
				// �f�[�^�T�C�Y�`�F�b�N
				if (sect_header.wSize == sizeof(save_gpssns)) {
					// �f�[�^�T�C�YOK
					bErrFlag = NP_TRUE;
				}
				break;
			}

			// ����̃Z�N�V�����̃f�[�^����΂�
			fseek(file, sect_header.wSize, SEEK_CUR) ;

		}while (true);
	}

	if ( NP_TRUE == bErrFlag ) {
		// GPSSNS�Z�N�V�����w�b�_�ǂݍ��ݐ���
		// �t���O������
		bErrFlag = NP_FALSE;
		// GPSSNS�f�[�^�ǂݍ���
		fread(&save_gpssns, sizeof(save_gpssns), 1, file);
	}

	if ( NP_TRUE == bErrFlag ) {
		//// LOCSAVE.DAT�t�@�C�����ǂݍ��ݐ���
		///* �Z���T�o�͕��ʃf�[�^ */
		//FLOAT fDirection = 0.0F;
	 //   /* ���ʂ��i�r���ʂ���A�k0�x���v���ɕϊ����� */
		//fDirection = 90.0F - (static_cast< FLOAT >(save_gpssns.nHeading) / DEG2WORD);
	 //   /* ���ʏC��: 0.0[deg]�`360.0[deg] */
	 //   MOD(fDirection, 0.0F, 360.0F);
		///* �Z���T�o�͕��ʃZ�b�g */
		//SensorBackupBasic.Gyro.Direction = fDirection;

		///* �Z���T�o�͌X�Ίp�x�Z�b�g */
		//SensorBackupExtension.GSns.SlopeAngle = static_cast< FLOAT >(save_gpssns.nIncAngle) / 100.0F;

		///* �Z���T�g���b�v���[�^�Z�b�g */
		//SnsBkM.Odometer = save_gpssns.dTripMeter;
	}

	// �t�@�C���N���[�Y
	fclose( file );
	return;

}


int main()
{
	GetLocSaveData();

	return 0;
}