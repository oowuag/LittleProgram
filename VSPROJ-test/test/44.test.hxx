
#include <stdio.h>
#include <Windows.h>

#include <iostream>
using namespace std;


/** ���[���X�e�[�^�X���p��A (LOC) */
typedef union {
    struct {
        DWORD   GpsOk:1;                        ///< GPS�f�[�^�g�p�\���f (1:OK)
        DWORD   GyroOk:1;                       ///< �W���C���f�[�^�g�p�\���f (1:OK)
        DWORD   PulseOk:1;                      ///< �p���X�f�[�^�g�p�\���f (1:OK)
        DWORD   GsnsOk:1;                       ///< G�Z���T�f�[�^�g�p�\���f (1:OK)
        DWORD   Differential:1;                 ///< GPS�f�B�t�@�����V���� (1:differntial)
        DWORD   GpsDimension:2;                 ///< GPS���ʎ��� (00:0D, 01:1D, 10:2D, 11:3D)
        DWORD   DirOk:1;                        ///< ���ʎg�p�\���f (1:OK)
        DWORD   DirSource:1;                    ///< ���ʂ̏o�͌� (0:Gyro, 1:GPS)
        DWORD   VelOk:1;                        ///< ���x�g�p�\���f (1:OK)
        DWORD   VelSource:2;                    ///< ���x�̏o�͌� (0:Pulse, 1:GPS, 2:G-Sensor)
        DWORD   MovSource:2;                    ///< �ړ��ʂ̏o�͌� (0:Pulse, 1:GPS, 2:G-Sensor)
        DWORD   LR:2;                           ///< �E���ܔ��f (1:�E��, 2:����)
        DWORD   Inclination:2;                  ///< �X�΃X�e�[�^�X (0:�X�Δ��f�s�\, 1:UP, 2:DOWN, 3:LEVEL)
        DWORD   Back:1;                         ///< �o�b�N���ڑ��ɂ��o�b�N�����o (0:�O�i, 1:���)
        DWORD   GpsNoSignal:1;                  ///< GPS�q���̓d�g��S����M�ł��Ă��Ȃ�
        DWORD   GpsMultiPath:1;                 ///< GPS���ʃf�[�^�Ƀ}���`�p�X�̉e�������o (1:���o)
        DWORD   PulseError:1;                   ///< �ԑ��p���X�ُ� (1:�ُ�)
        DWORD   DragonNG:1;                     ///< GPS chip�Ƃ̐ڑ��ُ� (0:����, 1:�ُ�)
        DWORD   UpdateInterval:2;               ///< ���ʎ��� (0:2Hz, 1:4Hz, 2:5Hz, 3:10Hz)
        DWORD   ReqPosMail:1;                   ///< ���݂̃f�[�^���o��ԃX�e�[�^�X (1:out)
        DWORD   GpsSA:1;                        ///< SA�t���O�i0:SA�Ȃ�, 1:SA����j
        DWORD   WeavingAlarm:2;                 ///< �ӂ���x�� (0:�x���Ȃ� 1�`3:�x�����x��1�`3)
        DWORD   CautionStatus:1;                ///< GPS�A�Z���T�n�G���[��� (0:����, 1:�ُ�)
        DWORD   GpsDelayCorrFunc:1;             ///< GPS���ʃf�B���C�␳�@�\���ڃt���O(0:�@�\�Ȃ� 1:�@�\����)
        DWORD   GpsDelayCorrect:1;              ///< GPS���ʃf�B���C�␳�t���O(0:�␳�Ȃ� 1:�␳����)
    } Condition;
    DWORD       AllStatus;                      ///< �S�̂̃X�e�[�^�X
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
