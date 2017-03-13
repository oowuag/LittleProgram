/*---------------------------------------------------------------------------*/
/*
 * @(#)$Id$ 
 * @(#) Declaration file of class HMI_CSS_CarSigInfoHandler.
 *
 * (c)  PIONEER CORPORATION  2010
 * 25-1 Nishi-machi Yamada Kawagoe-shi Saitama-ken 350-8555 Japan
 * All Rights Reserved.
 */
/** @file   
 * Declaration of HMI_CSS_CarSigInfoHandler.
 * 
 * TO DESC : FILE DETAIL DESCRIPTION, BUT DON'T DESCRIBE CLASS DETAIL HERE.
 @attention C++ used only,C forbidden
 */
#ifndef CXX_HMI_CSS_CARSIGINFOHANDLER_H
#define CXX_HMI_CSS_CARSIGINFOHANDLER_H
#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

/*---------------------------------------------------------------------------*/
/*    Reading of an include file                                             */

#ifndef CXX_AIPI_HMICSSDATATYPEDEF_H
#	include "AIPI_HMICSSDataTypeDef.h"
#endif

/*---------------------------------------------------------------------------*/
/*    Front declaration of a class                                           */



/*---------------------------------------------------------------------------*/
/*    Local definition declaration                                           */



/*---------------------------------------------------------------------------*/

class HMI_CSS_CarSignalInfoSpeed
{
public:
	HMI_CSS_CarSignalInfoSpeed()
		:m_eSpeed(HMI_SRV_CSS_SPEED_INVALID)
	{
	}
	HMI_SRV_CSS_Speed GetSpecifySpeedStatus()
	{
		return m_eSpeed;
	}
	VOID SetSpecifySpeedStatus(HMI_SRV_CSS_Speed eSpeed);
	{
		m_eSpeed = eSpeed;
	}
protected:
private:
	HMI_SRV_CSS_Speed	m_eSpeed;
};

//-----------------------------------------------------------

class HMI_CSS_CarSignalInfoReverseShift
{
public:
	HMI_CSS_CarSignalInfoReverseShift()
		:m_eReverseShift(HMI_SRV_CSS_STATUS_INVALID)
	{
	}
	HMI_SRV_CSS_Status GetReverseShiftStatus()
	{
		return m_eReverseShift;
	}
	VOID SetReverseShiftStatus(HMI_SRV_CSS_Status eReverseShift)
	{
		m_eReverseShift = eReverseShift;
	}
protected:
private:
	HMI_SRV_CSS_Status m_eReverseShift;
};

//-----------------------------------------------------------

class HMI_CSS_CarSignalInfoParkingBrake
{
public:
	HMI_CSS_CarSignalInfoParkingBrake()
		:m_eParkingBreak(HMI_SRV_CSS_STATUS_INVALID)
	{
	}
	HMI_SRV_CSS_Status GetParkingBreakStatus()
	{
		return m_eParkingBreak;
	}
	VOID SetParkingBreakStatus(HMI_SRV_CSS_Status eParkingBreak)
	{
		m_eParkingBreak = eParkingBreak;
	}
protected:
private:
	HMI_SRV_CSS_Status m_eParkingBreak;
};

//-----------------------------------------------------------

class HMI_CSS_CarSignalInfoIllumination
{
public:
	HMI_CSS_CarSignalInfoIllumination()
		:m_eIllumination(HMI_SRV_CSS_STATUS_INVALID)
	{
	}
	HMI_SRV_CSS_Status GetCarIlluminationStatus()
	{
		return m_eIllumination;
	}
	VOID SetCarIlluminationStatus(HMI_SRV_CSS_Status eIllumination)
	{
		m_eIllumination = eIllumination;
	}

protected:
private:
	HMI_SRV_CSS_Status m_eIllumination;

};

//-----------------------------------------------------------

class HMI_CSS_CarSigInfoHandler : public HMI_CSS_CarSigInfoIF
{
public:
	HMI_CSS_CarSigInfoHandler();

	virtual HMI_SRV_CSS_Speed GetSpecifySpeedStatus();
	virtual HMI_SRV_CSS_Status GetParkingBreakStatus();
	virtual HMI_SRV_CSS_Status GetReverseShiftStatus();	
	virtual HMI_SRV_CSS_Status GetCarIlluminationStatus() : ;
	virtual NP_BOOL GetSpeed(FLOAT &fSpeed);
	virtual void OnAIPI_NaviReplyEvent(AIPI_NaviReplyEvent &rcEvent);
	virtual void OnCarSigInfoEvent(NCarSigInfoEvent &rnEv);

	void SpeedJudge();
	void UpdateReverseShiftStatus();
	NP_BOOL GetReverseShiftStatus(BYTE &byStatus);
//protected:
private:
	HMI_CSS_CarSignalInfoSpeed			m_cSpeed;
	HMI_CSS_CarSignalInfoReverseShift	m_cReverseShift;
	HMI_CSS_CarSignalInfoParkingBrake	m_cParkingBreak;
	HMI_CSS_CarSignalInfoIllumination	m_cIllumination;
	UI_ServiceUtility					*m_cServiceUtility;
	NP_BOOL								m_bChartEventEnabled; //?? do not need?

#ifdef COMP_OPT_FOR_PC
	FLOAT								m_fLocSpeed;
#endif

	FLOAT								m_fSpeed;
};


#endif /* CXX_HMI_CSS_CARSIGINFOHANDLER_H */
/* EOF */
