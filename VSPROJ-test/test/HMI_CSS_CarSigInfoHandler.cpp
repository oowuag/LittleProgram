/*---------------------------------------------------------------------------*/
/*
 * @(#)$Id$ 
 * @(#) Implementation file of class HMI_CSS_CarSigInfoHandler.
 *
 * (c)  PIONEER CORPORATION  2010
 * 25-1 Nishi-machi Yamada Kawagoe-shi Saitama-ken 350-8555 Japan
 * All Rights Reserved.
 */
/*---------------------------------------------------------------------------*/
/*    Reading of an include file                                             */

#ifndef CXX_HMI_CSS_CARSIGINFOHANDLER_H
#	include "HMI_CSS_CarSigInfoHandler.h"
#endif


/*---------------------------------------------------------------------------*/
static const CHAR g_szErrorFileInfo[] = "$Id: HMI_CSS_CarSigInfoHandler.cpp";


//-----------------------------------------------------------

//-----------------------------------------------------------

HMI_CSS_CarSigInfoHandler::HMI_CSS_CarSigInfoHandler()
:m_cSpeed()
,m_cReverseShift()
,m_cParkingBreak()
,m_cIllumination()
,m_cServiceUtility()
//m_bChartEventEnabled()
#ifdef COMP_OPT_FOR_PC
,m_fLocSpeed(0.0f)
#endif
,m_fSpeed(0.0f)
{
}

HMI_SRV_CSS_Speed 
HMI_CSS_CarSigInfoHandler::GetSpecifySpeedStatus()
{
	return m_cSpeed.GetSpecifySpeedStatus();
}


HMI_SRV_CSS_Status 
HMI_CSS_CarSigInfoHandler::GetParkingBreakStatus()
{
	return m_cParkingBreak.GetParkingBreakStatus();
}


HMI_SRV_CSS_Status 
HMI_CSS_CarSigInfoHandler::GetReverseShiftStatus()
{
	return m_cReverseShift.GetReverseShiftStatus();
}
	
HMI_SRV_CSS_Status 
HMI_CSS_CarSigInfoHandler::GetCarIlluminationStatus()
{
	return m_cIllumination.GetCarIlluminationStatus();
}


//Get Speed
NP_BOOL
HMI_CSS_CarSigInfoHandler::GetSpeed(FLOAT &fSpeed)
{


#ifdef COMP_OPT_FOR_PC

	NP_BOOL bRet = AL_ProcessInfo::IsAplConnected(AL_APL_ID_NAVI);
	if (NP_FALSE == bRet){
		return NP_FALSE;
	}
	
	fSpeed = m_fLocSpeed;
#else
	// 情報取得用の変数宣言
	NPGpsSystem cGpsSys;
	NPGpsSensorInfoStr sDataAdr;

	// エラーコード
	NP_ERROR err = cGpsSys.GyroGetSnsInfo(&sDataAdr);


	// エラー処理
	if (NP_NOERROR != err){
		AL_ErrorLog(APL_ERR_HMI_CSS_GET_SPD_FAIL, AL_ERROR_NO_OPTION);
		return NP_FALSE;
	}

	fSpeed = sDataAdr.Speed;
	
#endif

	return NP_TRUE;
}


/// AIPI receive Event
VOID 
HMI_CSS_CarSigInfoHandler::OnAIPI_NaviReplyEvent(const AIPI_NaviReplyEvent& rcEvent)
{
#ifdef COMP_OPT_FOR_PC
	switch (rcEvent.GetEventType()) {
		case AIPI_NAVI_REP_LOC_UPDATE:
			{
				const AIPI_NaviReplyLocUpdateEvent * pcEvent = reinterpret_cast<const AIPI_NaviReplyLocUpdateEvent *>(&rcEvent);
				AIPI_NAVI_LocInformation sLocInfo ={0};
				pcEvent->GetLocInformation(sLocInfo);
				if (NP_TRUE == sLocInfo.bVelocityOk)
				{
					m_fLocSpeed = sLocInfo.wRealVelocity * 0.1f;
				}
				else
				{
					m_fLocSpeed = 0.0f;
				}
				break;
			}
		default:
			break;
	}
#endif
}


/// CarSigInfoEvent 処理
VOID
HMI_CSS_CarSigInfoHandler::OnCarSigInfoEvent(const NCarSigInfoEvent &rnEv)
{
	AL_Log::Output(AL_LOG_KIND_HMI_SRV_CSS, "NCarSigInfoEvent Receive");
	// ParkingBreakとILLを取得
	HMI_SRV_CSS_Status eStatusPB = HMI_SRV_CSS_STATUS_INVALID;
	HMI_SRV_CSS_Status eStatusILL = HMI_SRV_CSS_STATUS_INVALID;

	// PB判断
	//! パーキングブレーキ<br>
	//! NCARSIG_INVALID: 無効<br>
	//! NCARSIG_SIDEBRK_OFF: OFF<br>
	//! NCARSIG_SIDEBRK_ON: ON
	if (NCARSIG_SIDEBRK_OFF == rnEv.side_brk_flg) {
		eStatusPB = HMI_SRV_CSS_STATUS_OFF;
	}
	else if (NCARSIG_SIDEBRK_ON == rnEv.side_brk_flg) {
		eStatusPB = HMI_SRV_CSS_STATUS_ON;
	}
	else {
		// do nothing
		;
	}

	// ILL判断
	//! NCARSIG_INVALID: 無効<br>
	//! NCARSIG_ILL_OFF: OFF<br>
	//! NCARSIG_ILL_ON: ON
	if (NCARSIG_ILL_OFF == rnEv.ill_flg) {
		eStatusILL = HMI_SRV_CSS_STATUS_OFF;
	}
	else if (NCARSIG_ILL_ON == rnEv.ill_flg) {
		eStatusILL = HMI_SRV_CSS_STATUS_ON;
	}
	else {
		// do nothing
		;
	}

	if (eStatusPB != GetParkingBreakStatus()) {
		m_cParkingBreak.SetParkingBreakStatus(eStatusPB);

		// 初期化中以外UIと別Processにイベントを発送
		if (NP_TRUE == m_bChartEventEnabled) {
			// ParkingBreak状態変更イベントを発行
			AIPI_HMIExtIF::NotifyHMICSSPBChanged(GetParkingBreakStatus());

			// UIへチャートイベントを発行
			m_cServiceUtility.PostChartEventInt(UIHMICARSENSOR::PARKING_BREAK_CHANGED_EVENT, GetParkingBreakStatus());
			AL_Log::Output(AL_LOG_KIND_HMI_SRV_CSS, "ParkingBreak: %d", GetParkingBreakStatus());
		}
		// 走行規制判定
		m_cCSSRestriction.RestrictionJudge(CSS_RESTRICTION_EVENT_PROCESS);
	}

	if (eStatusILL != GetCarIlluminationStatus()) {
		m_cIllumination.SetCarIlluminationStatus(eStatusILL);

		// 初期化中以外UIと別Processにイベントを発送
		if (NP_TRUE == m_bChartEventEnabled) {
			AIPI_HMIExtIF::NotifyHMICSSILLChanged(GetCarIlluminationStatus());

			// UIへチャートイベントを発行
			m_cServiceUtility.PostChartEventInt(UIHMICARSENSOR::CAR_ILLUMINATION_CHANGED_EVENT, GetCarIlluminationStatus());
			AL_Log::Output(AL_LOG_KIND_HMI_SRV_CSS, "Illumination: %d", GetCarIlluminationStatus());
		}
	}

}

/// 速度判断
VOID
HMI_CSS_CarSigInfoHandler::SpeedJudge()
{
	float	fThreshold = CSS_SPEED_THRESHOLD_1KM_PER_HOUR;
	
	// モデル判断
	if ((AL_APLINFO_MAKER_TOYOTA_MOP == m_eMakerType)||
		(AL_APLINFO_MAKER_TOYOTA_DOP == m_eMakerType))
	{
		// TOYOTAは8km/h
		fThreshold = CSS_SPEED_THRESHOLD_8KM_PER_HOUR;
	}
	

	FLOAT fSpeed = 0.0f;
	HMI_SRV_CSS_Speed eSpeedStatus = HMI_SRV_CSS_SPEED_INVALID;

	if (NP_FALSE == GetSpeed(fSpeed)) {
		m_cSpeed.SetSpecifySpeedStatus(HMI_SRV_CSS_SPEED_INVALID);
		return;
	}

	m_fSpeed = fSpeed;

	// 速度判断
	if (fThreshold > fSpeed) {
		eSpeedStatus = HMI_SRV_CSS_SPEED_BELOW;
	}
	else {
		eSpeedStatus = HMI_SRV_CSS_SPEED_OVER;
	}

	// 状態を更新
	if (eSpeedStatus != GetSpecifySpeedStatus()) {
		m_cSpeed.SetSpecifySpeedStatus(eSpeedStatus);

		// 初期化中以外はUIと別Processにイベントを発送
		if (NP_TRUE == m_bChartEventEnabled) {
			// AIPI eventを発行
			AIPI_HMIExtIF::NotifyHMICSSSpeedChanged(GetSpecifySpeedStatus());

			// UIへチャートイベントを発行
			m_cServiceUtility.PostChartEventInt(UIHMICARSENSOR::SPECIFY_SPEED_CHANGED_EVENT, GetSpecifySpeedStatus());
			AL_Log::Output(AL_LOG_KIND_HMI_SRV_CSS, "Speed: %d", GetSpecifySpeedStatus());
		}
	}

	return;
}


/// Reverse Shift状態更新
VOID
HMI_CSS_CarSigInfoHandler::UpdateReverseShiftStatus()
{
	// Entityから状態を取得
	BYTE byStatus = 0;
	HMI_SRV_CSS_Status eReverseShift = HMI_SRV_CSS_STATUS_INVALID;

	if (NP_FALSE == GetReverseShiftStatus(byStatus)) {
		m_cReverseShift.SetReverseShiftStatus(HMI_SRV_CSS_STATUS_INVALID);
		return ;
	}

	// 状態が変わるかを判断
	if (CSS_STATUS_OFF == byStatus) {
		eReverseShift = HMI_SRV_CSS_STATUS_OFF;
	}
	else if (CSS_STATUS_ON == byStatus) {
		eReverseShift = HMI_SRV_CSS_STATUS_ON;
	}
	else {
		// do nothing
	}

	// リバースシフト状態を更新
	if (eReverseShift != GetReverseShiftStatus()) {
		m_cReverseShift.SetReverseShiftStatus(eReverseShift);

		// 初期化中以外UIと別Processにイベントを発送
		if (NP_TRUE == m_bChartEventEnabled) {
			// send AIPI event
			AIPI_HMIExtIF::NotifyHMICSSReverseShiftChanged(GetReverseShiftStatus());

			// UIへチャートイベントを発行
			m_cServiceUtility.PostChartEventInt(UIHMICARSENSOR::REVERSE_SHIFT_CHANGED_EVENT, GetReverseShiftStatus());
			AL_Log::Output(AL_LOG_KIND_HMI_SRV_CSS, "ReverseShift: %d", GetReverseShiftStatus());
		}
	}

	return ;
}

	

// Reverse Shift状態を取得
NP_BOOL
HMI_CSS_CarSigInfoHandler::GetReverseShiftStatus(BYTE &byStatus)
{
	// 情報取得用の変数宣言
	NPGpsSystem cGpsSys;
	NPGpsGyroChkStr sDataAdr;

	// ジャイロセンサ自己診断データの取得
	NP_ERROR err = cGpsSys.GyroCheck(&sDataAdr);

	// エラー処理
	if (NP_NOERROR != err){
		AL_ErrorLog(APL_ERR_HMI_CSS_GET_RSS_FAIL, AL_ERROR_NO_OPTION);
		return NP_FALSE;
	}

	// バック信号状態Ｈ'：NP_GPS_BACK_SNS_H[1]、'Ｌ'：NP_GPS_BACK_SNS_L[0]
	if (NP_GPS_BACK_SNS_H == sDataAdr.back_sns) {
		byStatus = CSS_STATUS_ON;
	}
	else if (NP_GPS_BACK_SNS_L == sDataAdr.back_sns) {
		byStatus = CSS_STATUS_OFF;
	}
	else {
		byStatus = CSS_STATUS_INVALID;
	}

	return NP_TRUE;
}



/* EOF */
