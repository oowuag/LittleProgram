#include "Sns06.SRAM_Gps.h"

#include <Windows.h>
#include <math.h>



#define	SNS_GLOBAL
#define	SNS_GLOBAL_B

#define	GPS_GLOBAL
#define	GPS_GLOBAL_B

////
#define	TSNS_OFFSET_MAX		50		/* �¶ȥ��󥵥Хå����å׉������������ */
#define	TSNS_GAIN_MAX		20		/* �¶ȥ��󥵥Хå����å׉������������ */

#define	PRD_LEN_EX_BUF_SIZE	7		/* �ٶȤˌ�������xѧ���������Ø�����ΥХåե��� */
#define	PRD_LEN_CORR_BUF_SIZE	4		/* ���xѧ�������������S����{�Хåե��� */

/*** sns_model.h ***/
/* ���򤱄e�΄���Mode�ζ��x */
#define	AFTER			0x00		/* ��؜�uƷ */
#define	TOYOTA			0x01		/* TOYOTA��OEM */
#define	FORD			0x02		/* FORD��OEM */
#define	HONDA			0x03		/* HONDA��OEM */

/*** ��ǥ붨�x�� ***/
#define	NP_SNS_NAVI_UNKNOWN	0x0000		/* ��ǥ붨�x���� */
#define	NP_SNS_NAVI_OTHER	0x0001		/* �ض��α�Ҫ�ʤ� */
#define	NP_SNS_NAVI_JG500	0x0002		/* JG500/�Ϻ���܇ */

/* ȡ�����ǶȤζ��x 1/Cos(deg)��ӛ�����뤳�� */
#define	LEVEL			1.0F		/* ˮƽȡ�긶�� */
#define	SLANT			1.03528F	/* �Aбȡ�긶��(AVN) 1/Cos(15deg) */
#define	HONDA_SFEB		1.07114F	/* �Aбȡ�긶��(SFEB) 1/Cos(31-10deg) */
#define	HONDA_SJKA		1.00137F	/* �Aбȡ�긶��(SJKA) 1/Cos(13-10deg) */
#define	HONDA_SJDA		1.04569F	/* �Aбȡ�긶��(SDJA) 1/Cos(27-10deg) */
#define	HONDA_SDAX_KH		1.06418F	/* �Aбȡ�긶��(SDAX-KH) 1/Cos(30-10deg) */
#define	HONDA_SFEW		1.07114F	/* �Aбȡ�긶��(SFEW) 1/Cos(31-10deg) */


/* ����ѧ���ǩ`���͎�ǰ����`����� */
#define	ERR_INFO_NUM		10			/* ����ѧ���ǩ`���͎�ǰ����`����åХåե��� */


/* ���� */
#ifdef	PI
#   undef PI
#endif
#define	PI			3.1415926535897932L	/* �� */

#define	DEG2RAD			(PI/180.0)		/* deg����饸����ؤΉ�Q */
#define	RAD2DEG			(180.0/PI)		/* �饸���󤫤�deg�ؤΉ�Q */

#define	RAD2SEC			(RAD2DEG*3600.0)	/* �饸���󤫤���ؤΉ�Q */

#define	BYTE2DEG		(360.0F/256)		/* 360/256deg����deg�ؤΉ�Q */

#define	DEG2WORD		(65536/360.0F)		/* deg����360/65536deg�ؤΉ�Q */

#define	MAX_DAY_SEC		86400.0F 		/* Maximum SEC count in one day */
#define	MAX_WEEK_SEC		604800.0F 		/* Maximum SEC count in one week */
							/* 604800 = 7day*24hour*60min*60sec */
							/*        = 1week */
#define	MAX_WEEK_NUM		1024			/* 1 cycle of GPS week number */


/*** CAN-BUS�����EOL�O������v�B ***/
/* VSS�O���v�B */
#define	CAN_EOL_VSS_PTN			0x0000000F	/* EOL�O�� VSS�O�� */
#define	CAN_EOL_MSCAN_VSS		0x00000001	/* EOL�O�� MS-CAN */
#define	CAN_EOL_HARDWIRE_VSS		0x00000002	/* EOL�O�� �ϩ`�ɥ磻��(�ǥե����) */
#define	CAN_EOL_UNCONFIGURED_VSS	0x00000004	/* EOL�O�� δ���� */
#define	CAN_EOL_NOT_USED_VSS		0x00000008	/* EOL�O�� δʹ�� */
/* ��Щ`���O���v�B */
#define	CAN_EOL_REV_PTN			0x000000F0	/* EOL�O�� ��Щ`���O�� */
#define	CAN_EOL_MSCAN_REV		0x00000010	/* EOL�O�� MS-CAN */
#define	CAN_EOL_HARDWIRE_REV		0x00000020	/* EOL�O�� �ϩ`�ɥ磻�� */
#define	CAN_EOL_MSCAN_HARDWIRE_REV	0x00000040	/* EOL�O�� MS-CAN & �ϩ`�ɥ磻��(�ǥե����) */
#define	CAN_EOL_NOT_USED_REV		0x00000080	/* EOL�O�� δʹ�� */

/*** sns_bkup.h ***/

/* SNS�ΥХå����å׉�����׷�ӡ��������ƄӤ������Ϥϱؤ�
*  ��������Q���뤳��(��ʽ��0x������)
*/
#define	SNS_SRAM		0x080924	/* SNS SRAM ������(����) */
#define	SNS_SRAM_V001		0x080924	/* SNS SRAM ������(Ver.001�r��) */
#define	SNS_SRAM_V002		0x080924	/* SNS SRAM ������(Ver.002�r��) */
						/* ���ڥ�����Ȥ�+1���Ƥ���Щ`����� */
						/* �Хå����åץǩ`���������ʤ����ᡢVer.001�ȹ��� */

/*** ���󥵥ϥ�ɥ�����ä���SRAM���ꥢ�ˤĤ��� ***/
// SRAM�� 0��8KByte�Υ��ꥢ�򥻥󥵥ϥ�ɥ�ˤ����ä���
// ���Υ��ꥢ�򡢥��󥵥ϥ�ɥ���2�Ĥȥ���`�Ěs��1�Ĥ˷ָ���ä��Ф�
// ���󥵥ϥ�ɥ��äϕ����z�ߚ��ˡ�A��B��2�ĤΥ��������˕����z�ߤ��Ф�
// 2�ĳ֤Ĥ��Ȥǡ���������r�α��o���Ф���ΤȤ���
// ����`�Ěs�ϡ�����`�k���r�˕����z�ߤ��Ф���ΤȤ���
// SRAM���������äΥ��ե��åȂ������¤˶��x����
#define	SNS_LEARN_BK_DATA_A	0	/* ���󥵥ϥ�ɥ� ����ѧ���Хå����åץǩ`�� SRAM����������(A���������) */
#define	SNS_LEARN_BK_DATA_B	1024	/* ���󥵥ϥ�ɥ� ����ѧ���Хå����åץǩ`�� SRAM����������(B���������) */
#define	SNS_ERR_INFO_BK_DATA	2048	/* ���󥵥ϥ�ɥ� ����`�Ěs�Хå����åץǩ`�� SRAM���������� */




////
#define	NP_SNS_LOG_THREAD_WAKEUP		0x0F	/* ����åɄI���_ʼ */
#define	NP_SNS_LOG_THREAD_SLEEP			0x10	/* ����åɄI��K�� */
#define	NP_SNS_OFF_PROC_WAKEUP			0x11	/* OFF�I���_ʼ */
#define	NP_SNS_OFF_PROC_SLEEP			0x12	/* OFF�I��K�˽K�� */
#define	NP_SNS_BK_MEM_FROM_SRAM			0x13	/* SRAM����ѧ���ǩ`���͎��I����Фä��r�� */
#define	NP_SNS_BK_MEM_WAIT			0x14	/* ѧ���ǩ`���i���z�ߴ����I��K�˕r�� */

/* SRAM���ꥨ�ꥢ���쥯���ö��x */
#define	NP_SNS_A_SECTION	0x00			/* A��������� */
#define	NP_SNS_B_SECTION	0x01			/* B��������� */

/* SRAM���ꥨ�ꥢ������ */
#define NP_SNS_SECTION_AREA	1024			/* A,B���������Υ��ꥢ������ */

/*** gps_diag.h ***/
#define	SIGNATURE97		0x7242		/* TANS Original Value */
/*** tsk_gyro.cpp ***/
#define	SNS_SIGNATURE		SIGNATURE97+0x04	/* '04�� ���󥵩`��ʹ��(GIDRA��ǥ�) */


/* ��Щ`���� */
/* ��Щ`�����ζ��Ӥ���������Ƥ���늈R��״�B��ʾ����(DRAGON�ڲ��ǤϷ�ܞ����Τ�ע��) */
#define	REV_SNS_L		0		/* ��Щ`����������  0[V]�r�Υץ������Փ�� */
#define	REV_SNS_H		1		/* ��Щ`���������� 12[V]�r�Υץ������Փ�� */
#define	REV_UNKNOWN		2		/* ǰ�MՓ���� */

#define	ADS7828			0x04		/* TI�� 8ch 50kHz */
#define	XV_8100CB		0x0A		/* ���ץ���ȥ襳�� */
#define	LIS244ALH		0x08		/* STMicro */
#define	S_58LM20A		0x03		/* �������`���󥹥ĥ� */

/* ȡ�긶��λ�É�� �a���C�� */
#define	NON_CORR		0		/* �a���C��OFF */
#define	AUTO_CORR		1		/* �a����DRAGON�Ȥ��ԄӵĤˌg�� */
#define	MANUAL_CORR		2		/* �a���򥢥ץ�(��`���`)�η���Y���ˤ��g�� */

/* ܇�I�gװ�r��G����ȡ������ */
#define	UNKNOWN_SET		0		/* ���� */
#define	REVERSE_SET		1		/* ��ܞȡ���� */
#define	FRONT_SET		2		/* ����ȡ���� */

/* ܇�٥ѥ륹�ź����Դ / ��Щ`���ź����Դ */
#define	HARD_WIRED_ONLY		0		/* �ϩ`�ɥ磻��Τ� */
#define	CAN_BUS_ONLY		1		/* CAN-BUS�Τ� */
#define	HARD_WIRED_AND_CAN_BUS	2		/* �ϩ`�ɥ磻�䡢CAN-BUS �I���ӾA */
#define	NOT_CONNECT		3		/* �ϩ`�ɥ磻�䡢CAN-BUS �I��δ�ӾA */

// Lite �h��
#define	GSNS_BUILTIN		TRUE		/* G���󥵤��Пo */

#define	TSNS_BUILTIN		TRUE		/* �¶ȥ��󥵤��Пo */
#define	CAN_WINKER_INFO		FALSE		/* CAN-BUS����Υ����󥫩`�����Пo */
#define	WEAVING_DETECT		FALSE		/* �դ�Ĥ���֪ */

#define	PRODUCT_DEST		AFTER		/* ���� */
#define	INSTALL_GRADE		LEVEL		/* ȡ�����Ƕ� */
#define	DEFAULT_MOVE		FALSE		/* δѧ���Ǥ����Ф��_ʼ���뤫�� */
#define	FORWARD_LOGIC		REV_UNKNOWN	/* ǰ�MՓ�� */

#define	ADC_TYPE		ADS7828		/* A2D����Щ`���ηN� */
#define	GYRO_TYPE		XV_8100CB	/* ���㥤��ηN� */
#define	GSNS_TYPE		LIS244ALH	/* G-Sensor�ηN� */
#define	TSNS_TYPE		S_58LM20A	/* �¶ȥ��󥵤ηN� */

#define	TYRE_CHANGE		AUTO_CORR	/* �������⾶�仯�ʳ� */
#define	SETTING_CHANGE		NON_CORR	/* ȡ�����Ƕȉ���ʳ� */
#define	GSNS_INSTALL		REVERSE_SET	/* G����ȡ������ */

#define	F_SNS			CYCLE_5HZ	/* λ�ø������� [Hz] */

#define	VSS_SOURCE		HARD_WIRED_ONLY	/* ܇�٥ѥ륹�ź����Դ */
#define	REV_SOURCE		HARD_WIRED_ONLY	/* ��Щ`���ź����Դ */

/* ����ο����Ԥ��� by kato */
#define	PULSE_FILTER		FALSE		/* �ѥ륹�ե��륿��PSW OUT�˽ӾA����Ƥ��뤫�� */


////
/***  GYROѧ���å��귬�ť��`��  ***/
#define	GYRO_MEM_A		0x00		/* ���å���ʹ�� */
#define	GYRO_MEM_B		0x01		/* ���å���ʹ�� */



/* ܇�٥ѥ륹�v�B */
#define	HI_PULSE	0		/* ܇�٥ѥ륹�ե��륿��`�� ��ѥ륹܇ */
#define	LO_PULSE	1		/* ܇�٥ѥ륹�ե��륿��`�� 1��4�ѥ륹܇ */
#define	NORMAL_CORRECT	0		/* �ԄӾ��x�a����`�� */

/*** ���㥤���v�B ***/
/* ���㥤���ե��åȤ��v���붨�x */
#if (GYRO_TYPE == XV_8100CB)	/* ���ץ���ȥ襳��Έ��� */
#define	GYRO_OFFSET		1.35F		/* 0��늈R�� : 1.35[V] */
#define	GYOFF_DEF		(static_cast< FLOAT >(GYRO_OFFSET*4096.0F*25.0F/2.5F))
						/* ���㥤���ե��åȂ�[LSB/Block] */
#define	LIMIT_GYOFF		2048.0F		/* ���㥤����е�ΤȤꤦ�낎[LSB] */
						/* 1.35V����β�ւ� [LSB]  늈R�Q��50mV */
						/* 4096.0F/2.5F[V]/0.05F[V]*25 */
#else	/* ��������Υ��㥤��Έ��� */
#define	GYRO_OFFSET		2.5F		/* 0��늈R�� : 2.5[V] */
#define	GYOFF_DEF		(static_cast< FLOAT >(GYRO_OFFSET*4096.0F*25.0F/5.0F))
						/* ���㥤���ե��åȂ�[LSB/Block] */
#define	LIMIT_GYOFF		6144.0F		/* ���㥤����е�ΤȤꤦ�낎[LSB] */
						/* 2.5V����β�ւ� [LSB]  늈R�Q��300mV */
#endif
/* ���㥤��жȤ��v���붨�x */
#if (GYRO_TYPE == FAR_S1)	/* 02HDD-AVN�˴��d�θ�ʿͨ���㥤��Έ��� */
#define	GYRO_SENS		(0.025F*1.02F)	/* �ж� : 0.0255[V/deg/s] */
#elif (GYRO_TYPE == XV_8100CB)	/* ���ץ���ȥ襳��Έ��� */
#define	GYRO_SENS		0.0025F		/* �ж� : 0.0025[V/deg/s] */
#else 				/* �������Υ��㥤�� */
#define	GYRO_SENS		0.025F		/* �ж� : 0.025[V/deg/s] */
#endif

#if (GYRO_TYPE == XV_8100CB)	/* ���ץ���ȥ襳��Έ��� */
#define	INITIAL_GYRO_GAIN_VALUE (static_cast< FLOAT >(2.5F/4096.0F/GYRO_SENS/500.0F))
						/* ���㥤������[deg/s/LSB/Block] */
#else 				/* �������Υ��㥤�� */
#define	INITIAL_GYRO_GAIN_VALUE (static_cast< FLOAT >(5.0F/4096.0F/GYRO_SENS/500.0F))
						/* ���㥤������[deg/s/LSB/Block] */
#endif

#define	GAIN_DEF		(static_cast< FLOAT >(INITIAL_GYRO_GAIN_VALUE*INSTALL_GRADE))

#if (GYRO_TYPE == XV_8100CB)	/* ���ץ���ȥ襳��Έ��� */
#define	GYGAIN_MAX_VALUE	(INITIAL_GYRO_GAIN_VALUE*1.4F)	/* ���������� : +40% */
								/* (�A��45�� 30%+���ڥå�1.25%+ѧ���r�ΤФ�Ĥ�5%+��ԣ��) */
#define	GYGAIN_MIN_VALUE	(INITIAL_GYRO_GAIN_VALUE*0.9F)	/* ���������С�� : -10% */
								/* (���ڥå�1.25%+ѧ�����ΤФ�Ĥ�5%) */
#else 				/* �������Υ��㥤�� */
#define	GYGAIN_MAX_VALUE	(INITIAL_GYRO_GAIN_VALUE*1.5F)	/* ���������� : +50% */
								/* (�A��45�� 30%+���ڥå�10%+ѧ���r�ΤФ�Ĥ�5%+��ԣ��) */
#define	GYGAIN_MIN_VALUE	(INITIAL_GYRO_GAIN_VALUE*0.85F)	/* ���������С�� : -15% */
								/* (���ڥå�10%+ѧ�����ΤФ�Ĥ�5%) */
#endif

/* ���㥤��ֹͣ�ж���٥���v���붨�x */
#define	GYRO_OFF_LEVEL		40.0F		/* ֹͣ��٥���ڂ� */
#define	GYRO_SET_OK		GYRO_OFF_LEVEL	/* ֹͣ��٥�1 */
#define	GYRO_SET_MORE_OK	10.0F		/* ֹͣ��٥�2 */
#define	GYRO_SET_MOST_OK	5.0F		/* ֹͣ��٥�3 */

/*** G����ֹͣ�ж���٥���v���붨�x ***/
/* �����u or �|�ץ�A���u or ����늹��u or STMicro�u�Έ��� */
#if (GSNS_TYPE == MAS1390P) || (GSNS_TYPE == TPR70_GC5) || (GSNS_TYPE == AGS19001) || (GSNS_TYPE == LIS244ALH)
#define	GSNS_OFF_LEVEL		80.0F		/* ���ڂ�:80 (= 20.0*1.667*1.667+��ԣ) */
#define	GSNS_OFF_LEVEL_MAX	850.0F		/* ���:850 (= 300.0*1.667*1.667+��ԣ */
#define	GSNS_SET_OK		420.0F		/* ֹͣ��٥�1 (= 150.0*1.667*1.667+��ԣ) */
#define	GSNS_SET_MORE_OK	GSNS_OFF_LEVEL	/* ֹͣ��٥�2 */
#define	GSNS_SET_MOST_OK	20.0F		/* ֹͣ��٥�3 (= 7.0*1.667*1.667+��ԣ) */
#else	/* 02��ǥ���ǰ�Ζ|�ץ�G���󥵤Έ��� */
#define	GSNS_OFF_LEVEL		20.0F		/* ���ڂ�:20 */
#define	GSNS_OFF_LEVEL_MAX	300.0F		/* ���:300 */
#define	GSNS_SET_OK		150.0F		/* ֹͣ��٥�1 */
#define	GSNS_SET_MORE_OK	GSNS_OFF_LEVEL	/* ֹͣ��٥�2 */
#define	GSNS_SET_MOST_OK	7.0F		/* ֹͣ��٥�3 */
#endif

/*** G���󥵸жȤ��v���붨�x ***/
/* �����u or �|�ץ�A���u or ����늹��u�Έ��� */
#if (GSNS_TYPE == MAS1390P) || (GSNS_TYPE == TPR70_GC5) || (GSNS_TYPE == AGS19001)
#define	GSNS_SENS		1.0F		/* �ж� : 1.0[V/G] */
#else	/* 02��ǥ���ǰ�Ζ|�ץ�G���󥵡����������G���󥵤Έ��� */
#define	GSNS_SENS		0.6F		/* �ж� : 0.6[V/G] */
#endif

#define	G_A			9.80665F	/* �������ٶ�[m/s2] */

#if (GSNS_TYPE == LIS244ALH)	/* STMicroG���󥵤Έ��� */
#define	G_25LSB			((FLOAT)(GSNS_SENS*4096.0*25.0/2.5))
						/* ���֥�å���G���󥵸ж�[LSB/Block/G] */
#else	/* ���������G���󥵤Έ��� */
#define	G_25LSB			((FLOAT)(GSNS_SENS*4096.0*25.0/5.0))
						/* ���֥�å���G���󥵸ж�[LSB/Block/G] */
#endif

#define	G_GAIN			(G_A/G_25LSB)	/* G���󥵥�����[m/s2/LSB/Block] */

#define	GSNS_GAIN_DEF		INSTALL_GRADE	/* G���󥵥�����γ��ڂ� */

/* G���󥵥��ե��åȤ��v���붨�x */
#if (GSNS_TYPE == MAS1390P) || (GSNS_TYPE == AGS19001) /* �����uor����늹��u�Έ��� */
#define	GSNS_OFFSET		2.5F		/* 0��늈R�� : 2.5[V] */
#elif (GSNS_TYPE == TPR70_GC5)	/* 03��ǥ�|�ץ�G���󥵤Έ��� */
#define	GSNS_OFFSET		1.8F		/* 0��늈R�� : 1.8[V] */
#elif (GSNS_TYPE == LIS244ALH)	/* STMicroG���󥵤Έ��� */
#define	GSNS_OFFSET		1.5F		/* 0��늈R�� : 1.5[V] */
#else				/* 02��ǥ���ǰ�Ζ|�ץ�G���󥵤Έ��� */
#define	GSNS_OFFSET		2.0F		/* 0��늈R�� : 2.0[V] */
#endif

#if (GSNS_TYPE == LIS244ALH)	/* STMicroG���󥵤Έ��� */
#define	GSNS_OFS_DEF		(static_cast< FLOAT >(GSNS_OFFSET*4096.0F*25.0F/2.5F))	/* G���󥵥��ե��åȂ�[LSB/Block] */
#else				/* ���������G���󥵤Έ��� */
#define	GSNS_OFS_DEF		(static_cast< FLOAT >(GSNS_OFFSET*4096.0F*25.0F/5.0F))	/* G���󥵥��ե��åȂ�[LSB/Block] */
#endif

/* �¶ȥ����v�B */
#define	GAIN2WORD		(65535.0F/2.0F)	/* ������ - WORD��Q */
#define	TSNS_DEV_NUM		4		/* �¶Ȥ�ȡ�ä���ǥХ����� */
#define	TSNS_GYRO_OFFSET	0		/* ���㥤���ե��å� */
#define	TSNS_GYRO_GAIN		1		/* ���㥤������ */
#define	TSNS_GSNS_OFFSET	2		/* �ǥ��󥵥��ե��å� */
#define	TSNS_GSNS_GAIN		3		/* �ǥ��󥵥����� */
#define	TEMPERATURE_ERROR_NUM	1000.0F		/* �¶Ȯ�����[��] */



#define	EXCELLENT_DIM		3			/* ����������ѧ����ʹ�ä�������v���δ��� */ 
#define	ERR_INDEX_MAX		25.5F		/* �`��ָ����� */

/***  ���󥵥ꥻ�å�״�B��ȴ���`��  ***/
#define	SNS_RESET_OK		0x01		/* ����ꥻ�åȌg�� */
#define	SNS_RESET_NG		0x00		/* ����ꥻ�åȤ��Ƥ��ʤ� */

#define	GSNS_ANGLE		1		/* 3D�ϥ��֥�åȤ�ѧ�� */

/***  ����ȡ�����Ƕ��ö���  ***/
#define	CENTER_ANGLE		0		/* ���󥿩`��ȡ�����Ƕ� */
#define	RIGHT_ANGLE		1		/* ���򤭤�ȡ�����Ƕ� */
#define	LEFT_ANGLE		2		/* ���򤭤�ȡ�����Ƕ� */

#define	CHECK_OFF	0		/* ܇�٥ѥ륹�v�B�Хå����åץ����å����Ф�ʤ����� */

#define	GPS_PITCH_NG		100.0F		/* GPS�ԥå��Ǥ���ڻ����� */


/*** sns_model.h ***/

/* ��ǥ�e�΄���״�B�Θ����� */
typedef struct  {
    BYTE    GSnsBuiltin;		/* TRUE GSNS���� FALSE GSNS�ʤ� */
    BYTE    TSnsBuiltin;		/* TRUE TSNS���� FALSE TSNS�ʤ� */
    BYTE    CanWinkerInfo;		/* CAN-BUS����Υ����󥫩`��� TRUE ���� FALSE �ʤ� */
    BYTE    WeavingDetect;		/* TRUE �դ�Ĥ���֪���� FALSE �դ�Ĥ���֪�ʤ� */
    BYTE    Destination;		/* ���� */
    FLOAT   InstallGrade;		/* ȡ�긶���Ƕ� */
    BYTE    DefaultMove;		/* δѧ���Ǥ����Ф��_ʼ���뤫�� */
    BYTE    ForwardLogic;		/* ǰ�MՓ�� */
    BYTE    AdcChip;			/* A2D����Щ`���ηN� */
    BYTE    GyroDevice;			/* ���㥤��ηN� */
    BYTE    GSnsDevice;			/* G-Sensor�ηN� */
    BYTE    TSnsDevice;			/* �¶ȥ��󥵤ηN� */
    BYTE    TyreChange;			/* �������⾶�仯�ʳ����a������ */
    BYTE    SettingChange;		/* ȡ�����Ƕȉ���ʳ����a������ */
    BYTE    GSnsInstall;		/* G���󥵤�ȡ������ */
    BYTE    VssSource;			/* ܇�٥ѥ륹�ź����Դ */
    BYTE    RevSource;			/* ��Щ`���ź����Դ */
    BYTE    PulseFilter;		/* �ѥ륹�ե��륿��PSW OUT�˽ӾA����Ƥ��뤫�� TRUE �ӾA FALSE �ǽӾA */
} sSnsModelInfo;

/*** sns_bkup.h ***/

/*** ���󥵥Хå����åץǩ`������������ ***/
/* ���󥵳���ѧ��״�B�Ø���������� */
typedef struct  {
    BYTE    SnsAllOk;		/* ȫ����ʹ�ÿ��ܥե饰 */
    BYTE    ShopMode;		/* ���^չʾ��`�� */
    BYTE    SpeedOk;		/* ���xѧ����ʹ�ÿ��ܥե饰 0:ѧ����ʹ�ò��� 1:ѧ����ʹ�ÿ��� */
    BYTE    GyroOk;		/* ���㥤��ʹ�ÿ��ܥե饰 */
    BYTE    GSnsStatus;		/* G����ʹ�ÿ��ܥե饰 */
    BYTE    RevLineOk;		/* �Хå���ʹ�ÿ��ܥե饰 */
    BYTE    YawSetupValue;	/* Yaw�����ȡ�����Ƕ��O���� */
} sSnsLearnStatus;

/* ��`��ǩ`���Хå����å׉�������������� */
typedef struct  {
    DWORD   PulseCounter;	/* �ѥ륹������ */
    DOUBLE  Odometer;		/* ���ɥ�`�� 1��km�ǥ��`�󥪩`�Щ` ��`���ꥻ�åȤ��� */
    DOUBLE  ScheOdometer;	/* ��������`���å��ɥ�`�� 100��km�ǥ��`�󥪩`�Щ` */
    FLOAT   DeltaLatSum;	/* ��`������ŕr���Ƅ������� ���ȷ��� */
    FLOAT   DeltaLonSum;	/* �U�ȷ��� */
    FLOAT   DeltaDSum;		/* ���о��x */
    BYTE    DeltaSumOn;		/* �Ƅ�������g�ߥե饰 */
    DWORD   CanBusEndOfLineSetting;	/* CAN-BUS�����EOL(end-of-line)�O����� �Хå����å׉��� */
} sMailBackupData;

/* ܇�٥ѥ륹�Хå����å׉��������� �������֤����� */
typedef struct  {
    FLOAT   PeriodLength;	/* �a����ξ��xѧ����[m] */
    FLOAT   PeriodLengthGps;	/* GPS�a����ξ��xѧ����[m] */
    WORD    PeriodLengthCnt;	/* ���xѧ������������ 0:δѧ�� 1:���ܤΤ߽K�� 2��:ѧ���� */
    WORD    PrdLenCorrMode;	/* 1���ڤξ��x���a�������`�� MANUAL,NOMAL,FAST*/
    CHAR    PeriodDivision;	/* �ѥ륹�η�����(1, 2, 4, 8���ܤ�ʹ�ÿ���) */
    BYTE    PlsLineConnect;	/* FALSE:܇�٥ѥ륹δ�ӾA,  TRUE:܇�٥ѥ륹����*/
    FLOAT   ManualCorr;		/* MANUAL���x�a���S�� */
    BYTE    RevLineConnect;	/* ��Щ`�������ӾA����Ƥ��뤫��ʾ���ե饰 */
    BYTE    RevLogicSet;	/* ��Щ`������Փ�����åȤ���Ƥ��뤫��ʾ���ե饰 */
    BYTE    ForwardLogic;	/* ��Щ`������ǰ�MՓ�� */
} sSpeedBackupBasic;

/* ܇�٥ѥ륹�Хå����å׉��������� �������֤����� */
typedef struct  {
    FLOAT   MapMatchCorr;	/* MM���x�a���S�� */
    WORD    MapMatchCorrCnt;	/* MM distance correction counter */
    BYTE    PulseErr;		/* ܇�٥ѥ륹����`�ե饰 TRUE:����`�Ф� FALSE:����`�o�� */
    BYTE    PulseMode;		/* speed pulse filter mode */
    BYTE    PeriodLengthErr;	/* ܇�٥ѥ륹�v�B�Хå����åץǩ`��״�B�ե饰 TRUE:���� FALSE:���� */
    BYTE    LowSpeedNg;		/* ����܇�٥ѥ륹δ�k��܇�I�жϥե饰 */
    WORD    MinimumSpeedCnt;	/* ��ͳ����ٶ�ѧ������ */
    FLOAT   MinimumSpeed;	/* ��ͳ����ٶ�ѧ���� [m/sec] */
    FLOAT   LastPrdLen;		/* �������⾶�仯�ʳ� ǰ���a��ǰ���xѧ���� */
    WORD    LastPrdLenCnt;	/* ǰ���a��ǰ���xѧ������ */
    SHORT   TyreErrCnt;		/* �ʳ��ж��å����� */
    FLOAT   PhantomPrdLen;	/* �Yѧ����[m] */
    BYTE    PulseSignalError;	/* ܇�٥ѥ륹���� */
    BYTE    PastDivNum;		/* �^ȥ���ܱ�(JIS���ʡ����ܱȮ���܇�I��)
					   |= 0x02 : 2�ѥ륹܇�൱
					   |= 0x04 : 4�ѥ륹܇�൱
					   |= 0x08 : 8�ѥ륹܇�൱ */
} sSpeedBackupExtension;

/* ���㥤���󥵥Хå����å׉��������� �������֤����� */
typedef struct  {
    FLOAT   Direction;		/* ���㥤���󥵷�λ�ǩ`�� */
    FLOAT   RightGain;		/* �ҥ����� */
    FLOAT   LeftGain;		/* �󥲥��� */
    FLOAT   CommonGain;		/* ���ҹ�ͨ������ */
    BYTE    RGainCnt;		/* �ҥ������ѧ���� */
    BYTE    LGainCnt;		/* �󥲥����ѧ���� */
    BYTE    RGainCorrCnt;	/* �ҥ�������a������ */
    BYTE    LGainCorrCnt;	/* �󥲥�����a������ */
}sGyroBackupBasic;

/* ���㥤���󥵥Хå����å׉��������� �������֤����� */
typedef struct  {
    BYTE    ExcellentQuality;	/* �����������a���v���Υ�����ƥ� 0:�v���ʤ� */
    SHORT   ExcellentWeek;	/* ���������Ȥ�ѧ�������L */
    DWORD   ExcellentLimit;	/* �����������a���νK�˕r�g */
    FLOAT   ExcellentFunction[4];	/* �����������a���λ؎������΂S�� 3���v���ˌ��� */
    FLOAT   ExcellentAve;	/* ����������ѧ���򤷤��Ȥ��Υ��ե��åȤ�ƽ�� */
    FLOAT   Offset;		/* ���ե��åȂ� */
    FLOAT   DirErr;		/* ��λ�`�� */
    FLOAT   DirErrRelief;	/* ��λ�`��(���`�Մ�����) */
    FLOAT   SigmaLevel;		/* Gyro ֹͣ�ж��Υ����ޤ�铂� */
    FLOAT   GainMovingAve[2];	/* ���¥��㥤����������ƽ���� */
    BYTE    SettingChangeCnt[2];	/* �O�ýǶȉ���ʳ������󥿩` */
    BYTE    SensitivityErrCnt;	/* �жȮ����ʳ��å����� */
    BYTE    GyroDeviceError;	/* Gyro���󥵮��� */
    BYTE    AdcDeviceError;	/* A/D����Щ`������ */
    BYTE    TsnsDeviceError;	/* �¶ȥ��󥵮��� */
}sGyroBackupExtension;

/* G���󥵥Хå����å׉��������� �������֤����� */
typedef struct  {
    BYTE    GSnsErr;		/* �O�÷����ж��ե饰(1: NG) */
    BYTE    SettingDir;		/* ǰ�᷽���ж��ե饰(0:����,1: ��ܞ,2: ����) */
    BYTE    DistanceSet;	/* ���xʹ�ÿ����жϥե饰 (TRUE: set) */
    FLOAT   Offset;		/* ���ե��åȂ�*/
    WORD    OffsetCnt;		/* ���ե��åȥ����� */
    BYTE    SettingSide;	/* ��ܞ�����ȡ�긶����(����:1, ����:2) */
    FLOAT   YawTheta;		/* Yaw�����ȡ�긶���Ƕ� */
    FLOAT   IncGain;		/* ���ٷ���Υ������a���� */
    FLOAT   DecGain;		/* �p�ٷ���Υ������a���� */
    WORD    IncGainCnt;		/* ���ٷ���Υ������a�������� */
    WORD    DecGainCnt;		/* �p�ٷ���Υ������a�������� */
}sGSnsBackupBasic;

/* G���󥵥Хå����å׉��������� �������֤����� */
typedef struct  {
    FLOAT   SigmaLevel;		/* G-Sensor ֹͣ�ж��Υ����ޤ�铂� */
    FLOAT   SlopeAngle;		/* �Aб�� ( down:[-], up:[+] ) */
    WORD    YawThetaCnt;	/* Yaw�����ȡ�긶���Ƕ�ѧ������ */
    BYTE    SettingSideCnt;	/* ��ܞ�����ȡ�긶����ѧ�������� */
    FLOAT   CommonGain;		/* �Ӝp�ٹ�ͨ������ */
    FLOAT   InstallPitchAngle;	/* �ԥå�����ȡ�긶���Ƕ� */
    BYTE    Reserve;		/* Reserve */
}sGSnsBackupExtension;

/* �¶ȥ��󥵥Хå����å׉��������� */
typedef struct  {
    BYTE    GyroOfsAve[TSNS_OFFSET_MAX];	/* ���㥤���ե��å�ƽ���������� */
    BYTE    GyroGainAve[TSNS_GAIN_MAX];		/* ���㥤������ƽ���������� */
    BYTE    GSnsOfsAve[TSNS_OFFSET_MAX];	/* G���󥵥��ե��å�ƽ���������� */
    BYTE    GSnsGainAve[TSNS_GAIN_MAX];		/* G���󥵥�����ƽ���������� */
    SHORT   GyroOffset[TSNS_OFFSET_MAX];	/* ���㥤���ե��å� */
    WORD    GyroGain[TSNS_GAIN_MAX];		/* ���㥤������ */
    SHORT   GSnsOffset[TSNS_OFFSET_MAX];	/* G���󥵥��ե��å� */
    WORD    GSnsGain[TSNS_GAIN_MAX];		/* G���󥵥����� */
} sTSnsBackupData;


/* ���󥵥Хå����å׉��������� �������֤����� */
typedef struct  {
    sSpeedBackupBasic		Speed;
    sGyroBackupBasic		Gyro;
    sGSnsBackupBasic		GSns;
} sSensorBackupBasic;

/* ���󥵥Хå����å׉��������� �������֤����� */
typedef struct  {
    sSpeedBackupExtension	Speed;
    sGyroBackupExtension	Gyro;
    sGSnsBackupExtension	GSns;
} sSensorBackupExtension;

/* �Aб�����a���`��Ҋ�e��� */
typedef struct  {
    FLOAT   DegreeOfSnsDirErrorPosi;		/* ���󥵷�λ�`��e��[DEG] */
    FLOAT   DegreeOfSnsDirErrorPosiAbs;		/* ���󥵷�λ�`��e��(�~����)[DEG] */
    FLOAT   DegreeOfSnsDirErrorNega;		/* ���󥵷�λ�`��e��[DEG] */
    FLOAT   DegreeOfSnsDirErrorNegaAbs;		/* ���󥵷�λ�`��e��(�~����)[DEG] */
    /* ȡ�����Ƕ��Ɯy���ӃAб�������a���� */
    FLOAT   SetupAngleDeg;			/* �Ɯyȡ�����ǶȤν~���� �gλ[��] */
} sSensorBackupSetupIncEstErr;

/* CanBus܇�٥ѥ륹�����ʳ��Ø���������� */
typedef struct  {
    BYTE    DivNumErrLevel;	/* ���ܱȮ�����٥�(JIS����) */
    BYTE    NonlinearLevel;	/* �Ǿ����ԥ�٥� */
    BYTE    StepwiseLevel;	/* �A��״��٥� */
    BYTE    AlternusLevel;	/* �����仯��٥� */
    BYTE    Reserve1;		/* Reserve */
    BYTE    Reserve2;		/* Reserve */
    BYTE    Reserve3;		/* Reserve */
    BYTE    Reserve4;		/* Reserve */
} sCanBusPulseErr;

/* �ٶȤˌ�������xѧ���������Ø���������� */
typedef struct  {
    FLOAT   PrdLength;		/* �ٶȤˌ�������xѧ���� */
    FLOAT   SpeedAve;		/* ���xѧ���r��ƽ���ٶ� */
    WORD    PrdLengthCnt;	/* ���ٶȤξ��xѧ������ */
} sPeriodLengthExtension;

/* ���xѧ�������������Ø���������� */
typedef struct  {
    FLOAT   PrdLengthCorr[PRD_LEN_CORR_BUF_SIZE];	/* ���xѧ�������������S����{�Хåե� */
    BYTE    PrdLengthProcess;	/* ���xѧ������������������Ʃ`���� */
    BYTE    UseBufCnt;		/* ���xѧ���������������ʹ�åХåե��� */
    BYTE    PrdLengthOk;	/* ���xѧ������������ʹ�ÿ��ܥե饰 */
} sApproxPeriodLengthData;

/***  ���󥵥Хå����åץǩ`����״�B�O���ö���  ***/
enum NPSnsSnsMemEnum{
    NP_SNS_MEM_CONDITION_NG = 0,	// ���󥵥��� ���ò���
    NP_SNS_MEM_CONDITION_OK = 1,	// ���󥵥��� ���ÿ���(ѧ�����Ф�)
    NP_SNS_MEM_CONDITION_MISSING = 2	// ���󥵥��� �͎��I��ʧ��(ѧ�����o��)
					// ���󥵥���͎��I��򤪤��ʤä����͎��Ǥ��ʤ��ä�����
} ;

/*** bios_gps.h ***/

/* GPSλ�Ø����� */
typedef	struct {
    WORD    StatusCode;		/* �yλ���Ʃ`���� */
    WORD    TrackSat;		/* �yλʹ�ÿ����l���� */
    DWORD   SvNum;		/* ��׽�l�Ƿ��� */
    LONG    Latitude;		/* ���� */
    LONG    Longitude;		/* �U�� */
    SHORT   Altitude;		/* �߶� */
    SHORT   Velocity;		/* ˮƽ������ٶ� */
    SHORT   Heading;		/* ˮƽ�����ٶȤη�λ */
    SHORT   Climb;		/* ��ֱ������ٶ� */
    BYTE    Hdop;		/* HDOPˮƽ���򾫶� */
    BYTE    Vdop;		/* VDOP�߶ȷ��򾫶� */
    WORD    Laxis;		/* �`������L�S */
    WORD    Saxis;		/* �`����Ҷ��S */
    WORD    LaDir;		/* �`���L�S�΃A�� */
    BYTE    VelAcc;		/* �ٶȤξ��� */
    BYTE    DirAcc;		/* ��λ�ξ��� */
    BYTE    LocalTime[6];	/* �F�ڕr�̥ǩ`�� */
    BYTE    FixDelay;		/* �yλ�r�̤�����W�� */
} sGpsPosData;

/* ��GPS�l�Ǥ���� */
typedef struct {
    BYTE    SvPrn;		/* �l�Ƿ��� */
    BYTE    SvStatus;		/* �l�Ǥβ�׽״�B */
    BYTE    Elevation;		/* ���� */
    BYTE    Azimuth;		/* ��λ�� */
    SHORT   Doppler;		/* �ɥåץ�`�ܲ��� */
    SHORT   SearchRange;	/* ���`���� */
    BYTE    SvLevel;		/* ���ť�٥� (AMU) */
} sGpsSatData;

/* GPS�l���������Type2 */
typedef	struct {
    sGpsSatData    CurrSat[12];		/* 12�l�Ƿ֤��l����� */
    BYTE    TrackSat;			/* ��׽�l���� */
    BYTE    Reserve[3];			/* �ꥶ�`�� */
} sGpsSatStatus2;

/*** gps_bios.h ***/

/*****************************************************/
/* GPS�yλ�Y�����{���똋����                       */
/*  1. ���󥵥ץ�����ڤ�ʹ�ä���GPS�ǩ`��         */
/*  2. LOC�������˶ɤ��ǩ`����Ԫ�ǩ`���Ȥ���ʹ�ä��� */
/*****************************************************/
typedef	struct  {
    sGpsPosData		GpsPosDataBuf;	/* GPSλ�Ø����� */
    sGpsSatStatus2	GpsSatDataBuf;	/* GPS�l��������� */
    DOUBLE  curr_pos_ecef[4];	/* �F��λ�� (ECEF����) */
    FLOAT   AdjPdop;		/* λ�äξ����ӻ��S�� (2D�r����K3D��VDOP��ʹ�ä���Ӌ�㣩*/
    FLOAT   hdop;		/* ˮƽ����ξ����ӻ��S�� */
    FLOAT   vdop;		/* ��ֱ����ξ����ӻ��S�� */
    FLOAT   Heading;		/* ��λ */ 
    FLOAT   Velocity;		/* 2��Ԫ�ٶ� [m/s] */
    FLOAT   Climb;		/* ��ֱ������ٶ� */
    DOUBLE  FixTime;		/* ���󥵤�ʹ�ä�����K�yλ�r�� */
    SHORT   Laxis;		/* �`���L�S�뾶 */
    SHORT   Saxis;		/* �`����S�뾶 */
    SHORT   Angle;		/* �`���L�S�A�� */
    BYTE    DgpsNow;		/* 0:GPS, 1:DGPS */
    BYTE    TS;			/* ��׽�l���� */
    BYTE    GpsDim;		/* �yλ��Ԫ [2: 2D�yλ, 3: 3D�yλ] */
    BYTE    CHMFix;		/* 0:ͨ���yλ, 1:Clock Hold Mode */
    BYTE    posfix_flag;	/* get position flag */
    BYTE    status_code;	/* system status code number */
    BYTE    FixUpdate;		/* �yλ�ǩ`�����åץǩ`�ȥե饰 �����󥵥��������ã�*/
				/* 1: �yλ�ǩ`�����åץǩ`�� */
				/* 2: �yλ�ǩ`��Ӌ���� (GPS������������) */
				/* 0: �yλ�ǩ`���ϥ��󥵥�������ʹ�Üg�� */
    BYTE    MaxAMU;		/* ��׽�Ф��l�Ǥ������ͣՂ� */
    BYTE    MaxAMUSv;		/* �����ͣՂ����l�Ƿ��� */
    BYTE    SnsGpsStatus;	/* ��������GPS�Μyλ���Ʃ`���� (���ԩ`) */
    BYTE    GpsSigStatus;	/* GPS����󥹥Ʃ`���� */
				/*   bit0  : 1=�l�Ǥ��źŤ�ȫ�����ŤǤ��ʤ� */
				/*   bit1-2: �ޥ���ѥ��ʳ���٥�           */
				/*   bit3  : 1=SA����, 0=SA�ʤ�             */
				/*   bit4-7: ���                           */
    DWORD   GpsMultiStatus;	/* GPS�ΥǥХå���󥹥Ʃ`���� */
				/*  �ǥХå��äΚ��å��Ʃ`������� */
				/*  ���ӥåȤ���ζ������������Ϥ�����ޤ��� */
} sGpsSendData;

/* ���󥵥ץ�����ʹ�ä���GPS�ǩ`�������� */
typedef struct  {
    sGpsSendData *Mail;			/* GPS�yλ�Y�����{���똋���� */
    FLOAT   Velocity3D;			/* 3��Ԫ�ٶ� */
    FLOAT   PitchAngle;			/* GPS�ٶȤΥԥå��� */
    FLOAT   LastVelocity;		/* GPS��ǰ���ٶ� */
    FLOAT   LastHeading;		/* GPS��ǰ�ط�λ */
    FLOAT   LastFixTime;		/* GPS��ǰ�؜yλ�r�� */
    FLOAT   DelayTime;			/* GPS�Μyλ�r�g����Υǥ��쥤 sdp->t_meas_gyro - fix_time[sec] */
    BYTE    FixContinue;		/* GPS�B�A�yλ�ж� */
} sGpsInSns;

/* sns_handle�v�B */
#define	SPPRD_BUF	5		/* �ѥ륹���ڸ�{�Хåե�����(CPLD) */
#define	GYRO_CH		0		/* A/D����Щ`���Υ����ͥ� ���㥤�� (AIN1) */
#define	GSNS_CH		1		/* A/D����Щ`���Υ����ͥ� G���� (AIN2) */
#define	TSNS_CH		2		/* A/D����Щ`���Υ����ͥ� �¶ȥ��� (AIN4) */
#define	NUM_AD_SUM	3		/* �e�㤹��A/D�����ͥ��� GYRO,GSNS,TSNS */

/*** sns_gyro.h ***/
/* �����ƥ��v�B */
#define	SNS_BLOCK	50			/* 1�֥�å����L�� 50[msec] */
#define	SNS_BLOCK_SEC	0.05F			/* 1�֥�å����L�� 0.05[sec]*/
#define	F_BLOCK		(1000/SNS_BLOCK)	/* 1���g�˄I����֥�å��ǩ`�� 20[��] or [Hz] */
#define	SNS_BUF		(F_BLOCK/F_SNS)		/* 1���ǩ`���ǄI����֥�å��� */
#define	SNS_BUF_MSEC	(SNS_BUF*SNS_BLOCK)	/* 1���ǩ`�����L�� 50 * SNS_BUF[msec] */
#define	SNS_BUF_SEC	(SNS_BUF_MSEC/1000.0F)	/* 1���ǩ`�����L�� 0.05 * SNS_BUF[sec] */

#define	CYCLE_5HZ		5		/* 5Hz���� */

/* ���󥵤Υ֥�å����ǩ`�� (50ms) */
typedef struct  {
    LONG    AdSum[NUM_AD_SUM];		/* 1�֥�å��g��A/D�ηe�ゎ GYRO, GSNS, TSNS */
    SHORT   SpPulseCnt;			/* 1�֥�å��ФΥѥ륹�� */
    DWORD   SpPeriod[SPPRD_BUF];	/* �ѥ륹���� */
    BYTE    SpPeriodCnt;		/* �ѥ륹������ */
    BYTE    ReverseSense;		/* ��Щ`���������Ӥ�״�B 1:�Хå��ź� High, 0:�Хå��ź� Low */
    CHAR    AdjustMsec;			/* Adjust Msec(intms�򤺤餷���r:�yλ��)���k����������ʾ���ե饰 */
					/* SnsMsecCounter�γ��ڂ�����뤹�� -1 or 1 */
} sSnsBlock;

/* �������ǩ`�� ������ */
typedef struct  {
    sSnsBlock BlockData[SNS_BUF];	/* �֥�å����ǩ`�� */
    DOUBLE  TimeMeasure;		/* ��������ȡ�Õr�� [time of week] */
    DWORD   SnsSec;			/* ��������ȡ�Õr�� [sec] */
    WORD    SnsMsec;			/* ��������ȡ�Õr�� [msec] */
    CHAR    PeriodDivision;		/* �ѥ륹�η�����(1, 2, 4, 8���ܤ�ʹ�ÿ���) */
					/*  -1: ���܉������, 1: ���ܟo�� */
					/*   2: 2����, 4: 4����, 8: 8���� */
    WORD    PulseCntSum;		/* ���ǩ`���Ф�ȫ�ѥ륹�� */
    BYTE    PulseRemainder;		/* ܇�٥ѥ륹��Ƭ���å��Τߗʳ��������Ȥ�ʾ���ե饰 */
    BYTE    PulseErrCnt;		/* �ѥ륹������`�Υ����� */
    BYTE    AdErrCnt[NUM_AD_SUM];	/* A/D����`�λ��� 0��SNS_BUF */
    FLOAT   AdAve[NUM_AD_SUM];		/* A/D����ƽ�� */
    FLOAT   AdSigma[NUM_AD_SUM];	/* A/D���Υ����ޤ΂� */
    DWORD   SnsTimer;			/* �ԴON��������ǩ`�������� */
    WORD    MinSpPlsLen[2];		/* ֹͣ���k�M�r�ˤ���������ٶȤΥѥ륹��[msec] */
    FLOAT   Temperature;		/* �¶ȥ��󥵤��¶� */
    BYTE    Winker;			/* �����󥫩`�ź�(���ҡ��o����Bit�Ǳ�F����ʹ�ä�����) */
} sSnsRawData;

/* �����ƄӾ��x���Ƕȉ仯���ηe��I�� */
typedef	struct  {
    FLOAT   DeltaD;			/* 1���ǩ`��(SNS_BUF * 50ms)�g��ˮƽ��������о��x[m] */
    FLOAT   DeltaX;			/* 1���ǩ`���g�νU�ȷ������о��x[m] */
    FLOAT   DeltaY;			/* 1���ǩ`���g�ξ��ȷ������о��x[m] */
    FLOAT   DeltaZ;			/* 1���ǩ`��(SNS_BUF * 50ms)�g�δ�ֱ��������о��x[m] ��δʹ�ã� */
    FLOAT   DeltaTheta;			/* 1���ǩ`��(SNS_BUF * 50ms)�g�νǶȉ仯�� */
    FLOAT   AngularVel;			/* ���ٶ� [deg/sec] (�rӋ�ؤ�:��) */
} sSnsDelta;

#define	SNS_BUF_CNT	60			/* ���ǩ`�����{����Хåե����� */
						/* ���󥵥ɥ饤�ФǤδ_����(5��) + �ީ`����(1���) */

/* ����ѧ���ǩ`���͎�ǰ����`��� */
typedef	struct  {
    BYTE    ErrNum;	/* ����`���� */
    DWORD   Value;	/* ����΂� */
    DWORD   Lines;	/* ����`���k�������з��� */
    CHAR    Id[82];	/* VERSION�Ƕ��x����RCS ID�����ФؤΥݥ��� */
} sStoreErrInfo;

class NPSnsHandlerBase
{

    /*** sns_model.h ***/

    SNS_GLOBAL	sSnsModelInfo	SnsModelInfo;		/* ��ǥ�e�΄���״�B */

    /*** sns_bkup.h ***/

    /***** ���󥵥Хå����åץǩ`�� ****/
    /*** ע�����1 ***/
    /* ���󥵥Хå����åץǩ`���ϡ��ԄӵĤ˥�������ä���뤿�� */
    /* SNS_SIGNWORD�����ɤΥ��ɥ쥹�����ä���뤫�狼��ʤ�       */
    /* SNS_SIGNWORD�����ؤ���8BYTE���饤����ä���Υ��ɥ쥹��  */
    /* ���ä����褦�ˤ��뤿����{���äΉ��������Ԥ��Ф�         */
    /* ���Ή����Խ��˥��󥵥Хå����å׉�����׷�Ӥ��Ф���         */

    /*** ע�����2 ***/
    /* BackupCheckSum�ޤǤϹ̶������äȤ��뤿�������ʤ�����     */

    /*** ע�����3 ***/
    /* ������׷�Ӥ��Фä����Ϥˤϡ�SRAM�����Ť��x���ʤ������� */
    /* ͬ�r�˥Хå����åץЩ`�����ζ��x�򤷤ʤ�������           */

    /*** ע�����4 ***/
    /* ���󥵥Хå����åץǩ`���κ�Ӌ�������ϡ�1024byte�ޤǤȤ��� */
    /* ���Τ��ᡢ������׷�Ӥ��Фä����Ϥˤϡ��������δ_�J���Ф��� */
    /* 1024byte�򳬤�����Ϥˤϡ��˽M�ߤ��پ����Ф���Ҫ����       */
    /* Version1�r��ǡ�845+3Byte����                              */

    SNS_GLOBAL	DOUBLE	bkAdjustmentData;		/* �{���É������g�Хå����å׉����ǤϤʤ��� */

    /* ���󥵥Хå����åץǩ`���ˉ�����׷�Ӥ�����Ϥˤϡ����¤Ή������Ҫ */
    /* ���������Щ`�����Ή����SRAM�����ŤΉ����SIMULATOR�Ό��� */
    /* �������� ���󥵥Хå����åץǩ`�� */
    SNS_GLOBAL_B	WORD	SNS_SIGNWORD;		/* SENSOR SIGNWORD */
    SNS_GLOBAL_B	DWORD	SnsSRamNow;		/* �F�ڤ�SNS-SRAM������ */
    SNS_GLOBAL_B	BYTE	SnsMemory;		/* �F��ʹ���ФΥ��󥵥���GYRO_MEM_A or GYRO_MEM_B */
    SNS_GLOBAL_B	BYTE	SnsBackupError;		/* �Хå����åץǩ`������`�ե饰 */
    SNS_GLOBAL_B	BYTE	BackupCheckSum;		/* �Хå����åץǩ`���Υ����å����� */

    /* ���󥵳���ѧ��״�B�Ø����� */
    SNS_GLOBAL_B	sSnsLearnStatus		SnsLearnStatus;

    /* ���󥵥Хå����å׉��������� �������� */
    SNS_GLOBAL_B	sSensorBackupBasic	SensorBackupBasic;

    /*** GPS�v�B�Хå����å׉��� ***/
    /* ���ȡ��U�ȡ��߶� */
    SNS_GLOBAL_B	DOUBLE	bkCurrPosLla[3];
    SNS_GLOBAL_B	WORD	bkWeekNumber;		/* �L���� */
    SNS_GLOBAL_B	DWORD	bkMsecOfWeek;		/* �L�r�g */

    /* �¶ȥ��󥵥Хå����å׉��������� */
    SNS_GLOBAL_B	sTSnsBackupData		TSnsBackupData;

    /* ���󥵥Хå����å׉��������� �������� */
    SNS_GLOBAL_B	sSensorBackupExtension	SensorBackupExtension;

    /* ��`��ǩ`���Хå����å׉��������� */
    SNS_GLOBAL_B	sMailBackupData		SnsBkM;
    
    /* �Aб�����a�����`��Ҋ�e����v�B������ */
    SNS_GLOBAL_B	sSensorBackupSetupIncEstErr	SnsBkIncEstErr;

    /* CanBus܇�٥ѥ륹�����ʳ��Ø����� */
    SNS_GLOBAL_B	sCanBusPulseErr		CanPulseErr;

    /* �ٶȤˌ�������xѧ���������Ø����� */
    SNS_GLOBAL_B	sPeriodLengthExtension	PeriodLengthEx[PRD_LEN_EX_BUF_SIZE];

    /* ���xѧ�������������Ø����� */
    SNS_GLOBAL_B	sApproxPeriodLengthData	ApxPeriodLengthData;

    SNS_GLOBAL_B	CHAR	sns_bat_sign;		/* signature for battery RAM */
    /* �����ޤ� ���󥵥Хå����åץǩ`�� */
    /* ��������ϡ����󥵥Хå����åץǩ`���δΤ����Ԥ����Ҫ��������� */

    /* MID_AVN�Ǥϡ�SRAM�ؤ�byte�����z�ߤ������ʤ�(write�����o��)       */
    /* ���Τ��ᡢSRAM�ؤΕ����z���äΥǩ`�����������{�������Ҫ������   */
    /* �ǩ`����������2byte�������{�������Ҫ������                      */
    /* ����memcpy�ǤΥ��ԩ`�򿼑]��4Byte�������{�����Ф���ΤȤ���      */
    SNS_GLOBAL_B	CHAR	bkAdjustmentBuff[3];	/* ���󥵥Хå����åץǩ`���������{���åХåե� */
    /* �����ޤǡ����󥵥Хå����åץǩ`���δΤ����Ԥ����Ҫ��������� */

    /* sns_main.c�v�B */
    SNS_GLOBAL	sSnsLearnStatus		*SnsBkL;	/* ���󥵳���ѧ��״�B�Ø�����ؤΥݥ��� */
    SNS_GLOBAL	sSensorBackupBasic	*SnsBkB;	/* ���󥵥Хå����å׉��������� �������֤ؤΥݥ��� */
    SNS_GLOBAL	sSensorBackupExtension	*SnsBkE;	/* ���󥵥Хå����å׉��������� �������֤ؤΥݥ��� */

    /* sns_main.c�v�B */
    SNS_GLOBAL	sSnsDelta	SnsDelta;	/* �����ƄӾ��x���Ƕȉ仯���ηe��I���� */

    /*** NPSnsHandlerBase.cpp ***/
    SNS_GLOBAL	sStoreErrInfo	StoreErrInfo[ERR_INFO_NUM];	/* ����ѧ���ǩ`���͎�ǰ����`����åХåե� */


    /*** gps_bkup.h ***/

    /**********************************************
    *	Geodetic System
    */

    GPS_GLOBAL_B	SHORT	bkGeodPrm;	/* Datum No. */
    GPS_GLOBAL_B	DOUBLE	bkLocalAxis;	/* �ʒ�������γ���뾶*/
    GPS_GLOBAL_B	DOUBLE	bkLocalFlat;	/* �ʒ�������γ���뾶*/
    GPS_GLOBAL_B	DOUBLE	bkDeltaA;	/* �ʒ�������γ���뾶WGS-84ϵ�Ȥβ�)	*/
    GPS_GLOBAL_B	DOUBLE	bkDeltaF;	/* �ʒ�������γ���뾶WGS-84ϵ�Ȥβ�)	*/
    GPS_GLOBAL_B	DOUBLE	bkEcc2;		/* f(2-f) */
    GPS_GLOBAL_B	DOUBLE	bkOneMe2;	/* 1-bkEcc2 */
    GPS_GLOBAL_B	DOUBLE	bkDeltaX;	/* ԭ��ƽ���Ƅ��� */
    GPS_GLOBAL_B	DOUBLE	bkDeltaY;	/* ԭ��ƽ���Ƅ��� */
    GPS_GLOBAL_B	DOUBLE	bkDeltaZ;	/* ԭ��ƽ���Ƅ��� */

    /*** pos_mail.cpp ***/

    DOUBLE	M2SecLat;		/* [m]->[sec]�ؤΉ�Q�ѥ��`�� */
    DOUBLE	M2SecLon;		/* [m]->[sec]�ؤΉ�Q�ѥ��`�� */
    DOUBLE	M2SecRefLat;		/* ��Q�ѥ��`����Ӌ�㤷������ [RAD] */
    BYTE	GpsLastPosOk;		/* �Є��ʜyλ�ǩ`�������� */
    DOUBLE	LastMailAltitude;	/* ��`��ǩ`��������˥��åȤ���GPS�߶�[m] */
    BYTE	MailDebugCnt;		/* ���ťǩ`���Υ����� 0-255 */
    BYTE	CautionStatusCnt;	/* GPS������ϵ����`����å����� */
    BYTE	GpsCautionStatus;	/* GPSϵ����`�����(0:����`�ʤ� 1:����`����) */

protected:
	    NPGpsStoreDataStatusStr *m_psDataStatus;            ///< API�ù��Х���Υݥ���
    sGpsSnsMail	GpsSnsMailBuff;			/* ��`��ǩ`��������('00) */
    WORD	LatestWeekNumber;	// �L����
    DWORD	LatestMsecOfWeek;	// �L�r�g

    NP_BOOL	NotifiedOfGpsFlag;		/* GPS�yλ�ǩ`����֪ͨ������Ƥ��뤳�Ȥ�ʾ���ե饰 */
    NP_BOOL	ErrorInfoCondition;		/* ����`�Ěs�͎�״�B�ե饰 */
    BYTE	StoreErrInfoNum;		/* ����ѧ���ǩ`������`��󱣴��åݥ��� */

	    NP_BOOL	MakeErrorInfoFlag;	// ����`�Ěs���¥ե饰
    NPSnsSnsMemEnum	SnsMemCondition;		/* ���󥵥Хå����åץǩ`��״�B */

    /*** sns_main.cpp ***/
    sGpsInSns		GpsInSns;		/* ���󥵥ץ�����ʹ�ä���GPS�ǩ`�������� */
    FLOAT		TmpGpsLastVelocity;	/* GPSǰ���ٶȤ����ɤ��뤿��Ή��� */
    FLOAT		TmpGpsLastHeading;	/* GPSǰ�ط�λ�����ɤ��뤿��Ή��� */
    FLOAT		TmpGpsLastFixTime;	/* GPSǰ�؜yλ�r�̤����ɤ��뤿��Ή��� */

    GPS_GLOBAL	sGpsSendData	GpsMailData;	/* LOC�ؤΥ�`�똋���� */
    GPS_GLOBAL	BYTE		GpsTestMode;	/* GPS Test Mode On flag */
    GPS_GLOBAL	BYTE		SimLog;		/* GPS/SNS Data output for Simulation */

    sSnsRawData	SnsRawData[SNS_BUF_CNT];	/* ���ǩ`�����{���똋���� */
    BYTE	RawDataReadPtr;			/* SnsRawData�������`�ɥݥ��� */
    BYTE	RawDataWritePtr;		/* SnsRawData������饤�ȥݥ��� */
    DWORD	LastSnsTimer;			/* ǰ�ؤΥ��󥵥����� */

	    WORD	AdjSnsMemSize;	/* 4BYTE�������{���g�ߥ��󥵥Хå����åץǩ`�������� */
				/* ���������å������ʤΤǌg�H�Υǩ`���������ȤϮ��ʤ�Τ�ע�� */
    /*** sns_decode.cpp ***/
    NP_BOOL		HaveGpsData;		/* Gps�ǩ`�����Ŝg�ߤǤ����¤�ե饰��ʾ�� */


	    SNS_GLOBAL	FLOAT	DeltaTheta;		/* 1���ǩ`��(SNS_BUF * 50ms)�g�νǶȉ仯�� */

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
	sSensorBackupBasic *bk_b  /* �Хå����åט����� �������֤ؤΥݥ��� */
	);
	VOID	InitGyroBackupBasic(
	sSensorBackupBasic *bk_b  /* �Хå����åט����� �������֤ؤΥݥ��� */
	);
	VOID	InitGSnsBackupBasic(
	sSensorBackupBasic *bk_b  /* �Хå����åט����� �������֤ؤΥݥ��� */
	);
	VOID	WriteToSnsBkSRAM( VOID );
	VOID	InitSpeedBackupExtension(
	sSensorBackupExtension *bk_e  /* �Хå����åט����� �������֤ؤΥݥ��� */
	);
	VOID	InitGyroBackupExtension(
	sSensorBackupExtension *bk_e  /* �Хå����åט����� �������֤ؤΥݥ��� */
	);
	VOID	InitGSnsBackupExtension(
	sSensorBackupExtension *bk_e  /* �Хå����åט����� �������֤ؤΥݥ��� */
	);
	VOID	InitSpeedAllParameters(VOID);
	VOID	InitSpeedPrms(
									 BYTE code
									 );
    VOID InitDirectPrms(
	LONG	elapsed_time,	/* ǰ���ԴOFF����νU�^�r�g [��] */
	SHORT	gps_week_num	/* ���ӕr��GPS�L���� */
	);
	VOID	InitGyroAllParameters(VOID);
	VOID	InitGSnsAllParameters(VOID);
	VOID	InitAccelPrms(VOID);
	VOID	InitTSnsAllParameters(VOID);
	VOID	InitSensorVariables(
	LONG elapsed_time,		/* ǰ���ԴOFF����νU�^�r�� �����*/
	SHORT gps_week_num		/* ���ӕr��GPS�L���� */
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
    DOUBLE	latitude	/* ���� [sec] */
    );

	VOID	OnInitialize(VOID);

};